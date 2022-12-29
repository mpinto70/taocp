#include "memory.h"

#include <algorithm>

namespace memory {

const Segment AVAIL{ 0, 0, reinterpret_cast<Segment*>(&Memory[0]) };

std::array<char, BUFFER_SIZE> Memory;

void init() {
    std::fill(Memory.begin(), Memory.end(), 0xf0);
    auto head = AVAIL.link;
    head->location = 0;
    head->size = Memory.size() - sizeof(Segment);
    head->link = nullptr;
}

void deinit() {}

}  // namespace memory
