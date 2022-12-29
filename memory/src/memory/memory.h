#pragma once

#include <cstddef>

namespace memory {

constexpr int BUFFER_SIZE = 300;
constexpr std::size_t NO_LINK = std::size_t(-1);

struct Segment {
    std::size_t size = 0;
    std::size_t link = NO_LINK;
};

extern const Segment AVAIL;

void init();
void deinit();

Segment* segment_at(std::size_t idx);

void* allocate(std::size_t size);

}  // namespace memory
