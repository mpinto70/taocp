#pragma once

#include <cstdint>
#include <limits>
#include <vector>

namespace memory {

using size_type = uint64_t;
constexpr size_type NO_LINK = std::numeric_limits<size_type>::max();

struct Segment {
    size_type location = 0;
    size_type size = 0;
    size_type link = NO_LINK;
};
using Segments = std::vector<Segment>;

void init(size_type memory_size);
void deinit();

void* allocate(size_type size);
void* allocate_filled(size_type size, unsigned char c);
bool deallocate(const void* ptr);

}  // namespace memory
