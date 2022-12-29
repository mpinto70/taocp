#include "memory/memory.h"

#include <gtest/gtest.h>

namespace memory {

class MemoryTest : public ::testing::Test {
public:
    MemoryTest() { init(); }
    ~MemoryTest() override { deinit(); }
};

void print_memory() {}

TEST_F(MemoryTest, SegmentConstruction) {
    constexpr Segment seg;

    EXPECT_EQ(seg.size, 0);
    EXPECT_EQ(seg.link, NO_LINK);
}

TEST_F(MemoryTest, InitState) {
    EXPECT_EQ(AVAIL.size, 0);
    EXPECT_EQ(AVAIL.link, 0);

    auto head = segment_at(AVAIL.link);
    EXPECT_EQ(head->size, BUFFER_SIZE - sizeof(Segment));
    EXPECT_EQ(head->link, NO_LINK);
}

TEST_F(MemoryTest, AllocateOne) {
    auto ptr = allocate(30);
    EXPECT_NE(ptr, nullptr);
    auto head = segment_at(AVAIL.link);
    EXPECT_EQ(head->size, BUFFER_SIZE - sizeof(Segment) - 30 - sizeof(Segment));
    EXPECT_EQ(head->link, NO_LINK);
}

}  // namespace memory
