#include "memory.h"

#include <algorithm>
#include <cstddef>
#include <cstring>

namespace memory {

std::vector<std::byte> Memory;
std::vector<Segment> FreeSegments;
std::vector<Segment> UsedSegments;

bool before(const Segment& segment, size_type location) {
    return segment.location < location;
}

auto emplace_segment(
        std::vector<Segment>& segments,
        size_type location,
        size_type size,
        size_type link) {
    const auto pos = std::lower_bound(segments.begin(), segments.end(), location, before);
    return segments.emplace(pos, location, size, link);
}

auto find_last_smaller(std::vector<Segment>& segments, size_type location) {
    const auto pos = std::lower_bound(segments.begin(), segments.end(), location, before);
    if (pos == segments.begin()) {
        return segments.begin();
    } else {
        return pos - 1;
    }
}

void init(size_type memory_size) {
    Memory.resize(memory_size, std::byte{ '-' });
    FreeSegments = std::vector{ Segment{ 0, memory_size, NO_LINK } };
    UsedSegments.clear();
}

void deinit() {
    Memory.clear();
    FreeSegments.clear();
    UsedSegments.clear();
}

void* allocate(size_type size) {
    for (size_type i = 0; i < FreeSegments.size(); ++i) {
        auto& segment = FreeSegments[i];
        if (segment.size >= size) {
            size_type new_location = segment.location + segment.size - size;
            emplace_segment(UsedSegments, new_location, size, NO_LINK);
            segment.size -= size;
            if (segment.size == 0 && i != 0) {
                FreeSegments[i - 1].link = segment.link;
                FreeSegments.erase(FreeSegments.begin() + i);
            }
            return (&Memory[new_location]);
        }
    }

    return nullptr;
}

template <typename IT>
void coalesce_if_together(std::vector<Segment>& segments, IT before, IT after) {
    if (before->location + before->size == after->location) {
        before->size += after->size;
        before->link = after->link;
        segments.erase(after);
    }
}

bool deallocate(const void* ptr) {
    const size_type location = reinterpret_cast<const std::byte*>(ptr) - &Memory[0];
    const auto pos_used =
            std::lower_bound(UsedSegments.begin(), UsedSegments.end(), location, before);
    if (pos_used == UsedSegments.end() || pos_used->location != location) {
        return false;
    }

    // avoid invalidation of free_before upon insertion of new free segment
    FreeSegments.reserve(FreeSegments.size() + 1);
    const auto free_before = find_last_smaller(FreeSegments, location);
    if (free_before == FreeSegments.end()) {
        return false;
    }
    const auto free_after = free_before + 1;

    if (free_after != FreeSegments.end()
        && pos_used->location + pos_used->size == free_after->location) {
        free_after->location = pos_used->location;
        free_after->size += pos_used->size;
        free_before->link = free_after->location;
        coalesce_if_together(FreeSegments, free_before, free_after);
    } else if (free_before->location + free_before->size == pos_used->location) {
        free_before->size += pos_used->size;
        if (free_after != FreeSegments.end()) {
            coalesce_if_together(FreeSegments, free_before, free_after);
        }
    } else {
        auto inserted = FreeSegments.insert(free_after, *pos_used);

        inserted->link = free_before->link;
        free_before->link = inserted->location;
    }

    UsedSegments.erase(pos_used);

    return true;
}

void* allocate_filled(size_type size, unsigned char c) {
    auto res = reinterpret_cast<unsigned char*>(allocate(size));
    if (res != nullptr) {
        std::fill(res, res + size, c);
    }
    return res;
}

}  // namespace memory
