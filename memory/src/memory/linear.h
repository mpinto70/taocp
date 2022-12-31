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
    SizeType location = 0;
    SizeType size = 0;
};
using Segments = std::vector<Segment>;

void Init(SizeType memory_size);
void Deinit();

void* Allocate(SizeType size);
void* AllocateFilled(SizeType size, unsigned char c);
bool Deallocate(const void* ptr);

}  // namespace memory::linear
