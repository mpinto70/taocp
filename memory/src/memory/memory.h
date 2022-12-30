#pragma once

#include <cstdint>
#include <limits>

namespace memory {

using size_type = uint32_t;
constexpr size_type BUFFER_SIZE = 300;
constexpr size_type NO_LINK = std::numeric_limits<size_type>::max();

struct Segment {
    size_type location = 0;
    size_type size = 0;
    size_type link = NO_LINK;
};

void init();
void deinit();

void* allocate(size_type size);
void* allocate_filled(size_type size, unsigned char c);

}  // namespace memory
