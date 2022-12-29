#include "memory/memory.h"

#include <gtest/gtest.h>

#include <numeric>

namespace memory {

extern std::array<std::byte, BUFFER_SIZE> Memory;

void print_memory() {
    for (auto c : Memory) {
        printf("%02x ", std::to_integer<int>(c));
    }
    printf("\n");
}

class MemoryTest : public ::testing::Test {
public:
    MemoryTest() { init(); }
    ~MemoryTest() override { deinit(); }
};

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
    constexpr size_type size = 30;
    print_memory();
    auto ptr = reinterpret_cast<unsigned char*>(allocate(size));
    std::iota(ptr, ptr + size, 1);
    EXPECT_NE(ptr, nullptr);
    auto head = segment_at(AVAIL.link);
    EXPECT_EQ(head->size, BUFFER_SIZE - sizeof(Segment) - size - sizeof(Segment));
    EXPECT_EQ(head->link, NO_LINK);
    print_memory();
}

}  // namespace memory
