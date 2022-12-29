#include "memory.h"

#include <algorithm>
#include <array>
#include <cstddef>

namespace memory {

namespace {
std::array<std::byte, BUFFER_SIZE> Memory;
}

const Segment AVAIL{ 0, 0 };

void init() {
    std::fill(Memory.begin(), Memory.end(), std::byte{ 0x00 });
    auto head = segment_at(AVAIL.link);
    head->size = Memory.size() - sizeof(Segment);
    head->link = NO_LINK;
}

void deinit() {}

Segment* segment_at(size_type idx) {
    return reinterpret_cast<Segment*>(&Memory[idx]);
}

void* allocate(size_type size) {
    const size_type effective_size = size + sizeof(Segment);
    size_type q = 0;
    while (q != NO_LINK) {
        auto seg_q = segment_at(q);
        if (seg_q->size >= effective_size) {
            auto seg_w = segment_at(q - effective_size);
            seg_w->size = effective_size;
            seg_w->link = NO_LINK;
            seg_q->size -= effective_size;
            return (&seg_w[1]);
        }
        q = seg_q->link;
    }

    return nullptr;
}

}  // namespace memory
