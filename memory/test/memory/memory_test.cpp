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
    EXPECT_EQ(AVAIL.link, reinterpret_cast<Segment*>(&Memory[0]));
}

}  // namespace memory
