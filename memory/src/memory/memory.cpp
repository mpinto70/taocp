#include "memory.h"

#include <algorithm>
#include <array>
#include <cstddef>

namespace memory {

namespace {
std::array<std::byte, BUFFER_SIZE> Memory;
}

const Segment AVAIL{ 0, 0, reinterpret_cast<Segment*>(&Memory[0]) };

void init() {
    std::fill(Memory.begin(), Memory.end(), std::byte{ 0x00 });
    auto head = AVAIL.link;
    head->location = 0;
    head->size = Memory.size() - sizeof(Segment);
    head->link = nullptr;
}

void deinit() {}

}  // namespace memory
