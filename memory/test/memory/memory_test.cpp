#include "memory/memory.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>

namespace memory {

extern std::array<std::byte, BUFFER_SIZE> Memory;

void print_segment(size_type q, const Segment& segment) {
    printf("%d / %d / ", q, segment.size);
    if (segment.link != NO_LINK) {
        printf("%d", segment.link);
    } else {
        printf("nil");
    }
    printf(" [ ");
    for (size_type i = 0; i < segment.size; ++i) {
        size_type idx = q + sizeof(Segment) + i;
        const auto c = std::to_integer<unsigned char>(Memory[idx]);
        if (std::isprint(c)) {
            printf("%c", c);
        } else {
            printf("?");
        }
    }
    printf(" ]\n");
}

void print_memory() {
    size_type q = 0;
    while (q < BUFFER_SIZE) {
        const auto seg = segment_at(q);
        print_segment(q, seg);
        q += sizeof(Segment) + seg.size;
    }
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
    EXPECT_EQ(head.size, BUFFER_SIZE - sizeof(Segment));
    EXPECT_EQ(head.link, NO_LINK);
}

TEST_F(MemoryTest, Allocate) {
    constexpr size_type size = 30;
    size_type allocated = 0;
    while (true) {
        auto ptr = reinterpret_cast<unsigned char*>(allocate(size));
        if (ptr == nullptr) {
            break;
        }
        allocated += sizeof(Segment) + size;
        auto head = segment_at(AVAIL.link);
        EXPECT_EQ(head.size, BUFFER_SIZE - sizeof(Segment) - allocated);
        EXPECT_EQ(head.link, NO_LINK);
    }

    EXPECT_NE(allocated, 0);
}

}  // namespace memory
