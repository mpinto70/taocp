#include "memory/memory.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>

namespace memory {

extern std::array<std::byte, BUFFER_SIZE> Memory;
extern std::vector<Segment> FreeSegments;
extern std::vector<Segment> UsedSegments;

void print_segment(const Segment& segment) {
    printf("%d / %d / ", segment.location, segment.size);
    if (segment.link != NO_LINK) {
        printf("%d", segment.link);
    } else {
        printf("nil");
    }
    printf(" [ ");
    for (size_type i = 0; i < segment.size; ++i) {
        size_type idx = segment.location + i;
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
    size_type free_idx = 0;
    size_type used_idx = 0;
    while (free_idx < FreeSegments.size() || used_idx < UsedSegments.size()) {
        const Segment* free_seg =
                free_idx < FreeSegments.size() ? &FreeSegments.at(free_idx) : nullptr;
        const Segment* used_seg =
                used_idx < UsedSegments.size() ? &UsedSegments.at(used_idx) : nullptr;
        if (free_seg != nullptr && used_seg != nullptr) {
            if (free_seg->location <= used_seg->location) {
                print_segment(*free_seg);
                ++free_idx;
            } else {
                print_segment(*used_seg);
                ++used_idx;
            }
        } else if (free_seg != nullptr) {
            print_segment(*free_seg);
            ++free_idx;
        } else {
            print_segment(*used_seg);
            ++used_idx;
        }
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
    ASSERT_EQ(FreeSegments.size(), 1);
    const auto& segment = FreeSegments[0];
    EXPECT_EQ(segment.location, 0);
    EXPECT_EQ(segment.size, BUFFER_SIZE);
    EXPECT_EQ(segment.link, NO_LINK);
}

TEST_F(MemoryTest, Allocate) {
    print_memory();
    constexpr size_type size = 31;
    size_type allocated = 0;
    while (true) {
        auto ptr = reinterpret_cast<unsigned char*>(allocate(size));
        if (ptr == nullptr) {
            break;
        }
        allocated += size;
    }

    EXPECT_NE(allocated, 0);
    print_memory();
}

}  // namespace memory
