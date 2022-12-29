#pragma once

#include <array>

namespace memory {

constexpr int BUFFER_SIZE = 300;

struct Segment {
    int location = 0;
    int size = 0;
    Segment* link = nullptr;
};

extern const Segment AVAIL;

extern std::array<char, BUFFER_SIZE> Memory;

void init();
void deinit();

}  // namespace memory
