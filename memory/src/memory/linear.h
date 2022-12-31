#pragma once

#include "memory_utils.h"

#include <vector>

namespace memory::linear {

// Somewhat covers what is explained in section 2.5-A and 2.5-B.
// Note that, in the book, Knuth put the segment headers inside main memory,
// and counts on, the user to inform the address and size of occupied
// memory correctly. Since this is a risk, I put the free and occupied
// memory bookkeeping in two distinct vectors. This is why, the deallocate
// function only takes the address of memory being deallocated, and the
// Segment struct does not need a link element.
// The way memory is occupied and freed is the same as described in the
// book, though.

struct Segment {
    size_type location = 0;
    size_type size = 0;
};
using Segments = std::vector<Segment>;

void init(size_type memory_size);
void deinit();

void* allocate(size_type size);
void* allocate_filled(size_type size, unsigned char c);
bool deallocate(const void* ptr);

}  // namespace memory::linear
