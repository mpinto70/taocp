#pragma once

#include <cstdint>
#include <limits>

namespace memory {

using size_type = uint32_t;
constexpr size_type BUFFER_SIZE = 300;
constexpr size_type NO_LINK = std::numeric_limits<size_type>::max();

struct Segment {
    size_type size = 0;
    size_type link = NO_LINK;
};

extern const Segment AVAIL;

void init();
void deinit();

Segment* segment_at(size_type idx);

void* allocate(size_type size);

}  // namespace memory
