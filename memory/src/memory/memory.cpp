#include "memory.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>

namespace memory {

std::array<std::byte, BUFFER_SIZE> Memory;
std::vector<Segment> FreeSegments;
std::vector<Segment> UsedSegments;

void init() {
    std::fill(Memory.begin(), Memory.end(), std::byte{ '-' });
    FreeSegments = std::vector{ Segment{ 0, BUFFER_SIZE, NO_LINK } };
    UsedSegments.clear();
}

void deinit() {
    FreeSegments.clear();
    UsedSegments.clear();
}

Segment segment_at(size_type idx) {
    Segment res;
    std::memcpy(&res, &Memory[idx], sizeof(res));
    return res;
}

void* allocate(size_type size) {
    for (auto& seg_q : FreeSegments) {
        if (seg_q.size >= size) {
            size_type new_seg_idx = seg_q.location + seg_q.size - size;
            UsedSegments.emplace(UsedSegments.begin(), new_seg_idx, size, NO_LINK);
            seg_q.size -= size;
            return (&Memory[new_seg_idx]);
        }
    }

    return nullptr;
}

void* allocate_filled(size_type size, unsigned char c) {
    auto res = reinterpret_cast<unsigned char*>(allocate(size));
    if (res != nullptr) {
        std::fill(res, res + size, c);
    }
    return res;
}

}  // namespace memory
