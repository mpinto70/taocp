#include "memory/memory.h"

#include <gtest/gtest.h>

namespace memory {

class MemoryTest : public ::testing::Test {
public:
    MemoryTest() { init(); }
    ~MemoryTest() override { deinit(); }
};

TEST_F(MemoryTest, SegmentConstruction) {
    constexpr Segment seg;

    EXPECT_EQ(seg.location, 0);
    EXPECT_EQ(seg.size, 0);
    EXPECT_EQ(seg.link, nullptr);
}

TEST_F(MemoryTest, InitState) {
    EXPECT_EQ(AVAIL.location, 0);
    EXPECT_EQ(AVAIL.size, 0);
    ASSERT_NE(AVAIL.link, nullptr);

    auto head = AVAIL.link;
    EXPECT_EQ(head->location, 0);
    EXPECT_EQ(head->size, BUFFER_SIZE - sizeof(Segment));
    EXPECT_EQ(head->link, nullptr);
}

}  // namespace memory
