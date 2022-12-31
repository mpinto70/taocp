#include "linear.h"

#include <algorithm>
#include <cstring>

namespace memory::linear {

MemoryT Memory;
Segments FreeSegments;
Segments UsedSegments;

bool before(const Segment& segment, size_type location) {
    return segment.location < location;
}

auto emplace_segment(Segments& segments, size_type location, size_type size) {
    const auto pos = std::lower_bound(segments.begin(), segments.end(), location, before);
    return segments.emplace(pos, location, size);
}

auto find_last_smaller(Segments& segments, size_type location) {
    const auto pos = std::lower_bound(segments.begin(), segments.end(), location, before);
    if (pos == segments.begin()) {
        return segments.begin();
    } else {
        return pos - 1;
    }
}

void init(size_type memory_size) {
    Memory.resize(memory_size, Byte{ '-' });
    FreeSegments = std::vector{ Segment{ 0, memory_size } };
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
            emplace_segment(UsedSegments, new_location, size);
            segment.size -= size;
            if (segment.size == 0 && i != 0) {
                FreeSegments.erase(FreeSegments.begin() + i);
            }
            return (&Memory[new_location]);
        }
    }

    return nullptr;
}

template <typename IT>
void coalesce_if_together(Segments& segments, IT before, IT after) {
    if (before->location + before->size == after->location) {
        before->size += after->size;
        segments.erase(after);
    }
}

bool deallocate(const void* ptr) {
    const size_type location = reinterpret_cast<const Byte*>(ptr) - &Memory[0];
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
        coalesce_if_together(FreeSegments, free_before, free_after);
    } else if (free_before->location + free_before->size == pos_used->location) {
        free_before->size += pos_used->size;
        if (free_after != FreeSegments.end()) {
            coalesce_if_together(FreeSegments, free_before, free_after);
        }
    } else {
        FreeSegments.insert(free_after, *pos_used);
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

}  // namespace memory::linear
