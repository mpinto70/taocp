#pragma once

namespace memory {

constexpr int BUFFER_SIZE = 300;

struct Segment {
    int location = 0;
    int size = 0;
    Segment* link = nullptr;
};

extern const Segment AVAIL;

void init();
void deinit();

}  // namespace memory
