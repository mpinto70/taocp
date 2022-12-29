#include "memory.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>

namespace memory {

std::array<std::byte, BUFFER_SIZE> Memory;

namespace {
void put_segment(size_type idx, const Segment& segment) {
    std::memcpy(&Memory[idx], &segment, sizeof(segment));
}
}  // namespace

const Segment AVAIL{ 0, 0 };

void init() {
    std::fill(Memory.begin(), Memory.end(), std::byte{ '-' });
    auto head = segment_at(AVAIL.link);
    head.size = Memory.size() - sizeof(Segment);
    head.link = NO_LINK;
    put_segment(AVAIL.link, head);
}

void deinit() {}

Segment segment_at(size_type idx) {
    Segment res;
    std::memcpy(&res, &Memory[idx], sizeof(res));
    return res;
}

void* allocate(size_type size) {
    const size_type effective_size = size + sizeof(Segment);
    size_type q = 0;
    while (q != NO_LINK) {
        auto seg_q = segment_at(q);
        if (seg_q.size >= effective_size) {
            size_type new_seg_idx = q + seg_q.size + sizeof(Segment) - effective_size;
            auto new_seg = segment_at(new_seg_idx);
            new_seg.size = size;
            new_seg.link = NO_LINK;
            seg_q.size -= effective_size;

            put_segment(q, seg_q);
            put_segment(new_seg_idx, new_seg);

            return (&Memory[new_seg_idx + sizeof(Segment)]);
        }
        q = seg_q.link;
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
