#include "memory/memory.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>
#include <random>

namespace memory {

extern std::array<std::byte, BUFFER_SIZE> Memory;
extern std::vector<Segment> FreeSegments;
extern std::vector<Segment> UsedSegments;

std::ostream& operator<<(std::ostream& out, const Segment& segment) {
    out << segment.location << " / " << segment.size << " / "
        << ((segment.link == NO_LINK) ? "nil" : std::to_string(segment.link));

    return out;
}

bool operator==(const Segment& lhs, const Segment& rhs) {
    return lhs.location == rhs.location && lhs.size == rhs.size && lhs.link == rhs.link;
}

bool operator!=(const Segment& lhs, const Segment& rhs) {
    return !(lhs == rhs);
}

size_type random_value(size_type min, size_type max) {
    static std::mt19937 gen((std::random_device())());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

void print_segment(const Segment& segment, char s) {
    printf("%c - %d / %d / ", s, segment.location, segment.size);
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

void print_segments(const std::string& name) {
    printf("++++++++++ %s ++++++++++\n", name.c_str());
    size_type free_idx = 0;
    size_type used_idx = 0;
    while (free_idx < FreeSegments.size() || used_idx < UsedSegments.size()) {
        const Segment* free_seg =
                free_idx < FreeSegments.size() ? &FreeSegments.at(free_idx) : nullptr;
        const Segment* used_seg =
                used_idx < UsedSegments.size() ? &UsedSegments.at(used_idx) : nullptr;
        if (free_seg != nullptr && used_seg != nullptr) {
            if (free_seg->location <= used_seg->location) {
                print_segment(*free_seg, 'f');
                ++free_idx;
            } else {
                print_segment(*used_seg, 'u');
                ++used_idx;
            }
        } else if (free_seg != nullptr) {
            print_segment(*free_seg, 'f');
            ++free_idx;
        } else {
            print_segment(*used_seg, 'u');
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

    EXPECT_EQ(seg.location, 0);
    EXPECT_EQ(seg.size, 0);
    EXPECT_EQ(seg.link, NO_LINK);
}

TEST_F(MemoryTest, InitState) {
    ASSERT_EQ(FreeSegments.size(), 1);
    const auto& segment = FreeSegments.at(0);
    EXPECT_EQ(segment.location, 0);
    EXPECT_EQ(segment.size, BUFFER_SIZE);
    EXPECT_EQ(segment.link, NO_LINK);
}

void CheckAllocate(
        size_type location,
        size_type size,
        const std::vector<Segment>& expected_free,
        const std::vector<Segment>& expected_used) {
    SCOPED_TRACE(location);
    EXPECT_NE(allocate_filled(size, 'x'), nullptr);

    // print_segments("allocated " + std::to_string(location));

    EXPECT_EQ(FreeSegments, expected_free);
    EXPECT_EQ(UsedSegments, expected_used);
}

TEST_F(MemoryTest, Allocate) {
    std::vector<Segment> expected_free = { Segment{ 0, BUFFER_SIZE, NO_LINK } };
    std::vector<Segment> expected_used;
    size_type location = BUFFER_SIZE;
    while (FreeSegments.at(0).size > 40) {
        const size_type size = random_value(30, 40);
        location -= size;
        expected_free.at(0).size -= size;
        expected_used.emplace(expected_used.begin(), location, size, NO_LINK);
        CheckAllocate(location, size, expected_free, expected_used);
    }

    EXPECT_FALSE(UsedSegments.empty());
}

void CheckDeallocate(
        size_type location,
        size_type size,
        const std::vector<Segment>& expected_free,
        const std::vector<Segment>& expected_used) {
    SCOPED_TRACE(location);
    std::fill_n(Memory.begin() + location, size, std::byte{ '.' });
    EXPECT_TRUE(deallocate(&Memory[location]));

    // print_segments("deallocated " + std::to_string(location));

    EXPECT_EQ(FreeSegments, expected_free);
    EXPECT_EQ(UsedSegments, expected_used);
}

TEST_F(MemoryTest, Deallocate) {
    constexpr size_type NUM_BLOCKS = 10;
    constexpr size_type size = BUFFER_SIZE / NUM_BLOCKS;
    size_type allocated = 0;
    while (true) {
        auto ptr = reinterpret_cast<unsigned char*>(allocate_filled(size, 'x'));
        if (ptr == nullptr) {
            break;
        }
        allocated += size;
    }
    EXPECT_EQ(allocated, NUM_BLOCKS * size);

    // print_segments("filled");

    auto expected_free = FreeSegments;
    auto expected_used = UsedSegments;
    size_type pos = 0;

    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX |

    // remove used space at 6 and create a new free space there
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | XXX | XXX | XXX | XXX | XXX | XXX | ... | XXX | XXX | XXX |
    pos = BUFFER_SIZE - (NUM_BLOCKS - 6) * size;
    expected_free.at(0).link = pos;
    expected_free.emplace_back(pos, size, NO_LINK);
    expected_used.erase(expected_used.begin() + 6);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space at 5 and coalesce 5-6
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | XXX | XXX | XXX | XXX | XXX | ... - ... | XXX | XXX | XXX |
    pos = BUFFER_SIZE - (NUM_BLOCKS - 5) * size;
    expected_free.at(0).link = pos;
    expected_free.at(1).location = pos;
    expected_free.at(1).size += size;
    expected_used.erase(expected_used.begin() + 5);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space at 0 and expand size of first free space
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... | XXX | XXX | XXX | XXX | ... - ... | XXX | XXX | XXX |
    pos = BUFFER_SIZE - (NUM_BLOCKS - 0) * size;
    expected_free.at(0).size += size;
    expected_used.erase(expected_used.begin() + 0);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space at 1 and expand size of first free space
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... | XXX | XXX | XXX | ... - ... | XXX | XXX | XXX |
    pos = BUFFER_SIZE - (NUM_BLOCKS - 1) * size;
    expected_free.at(0).size += size;
    expected_used.erase(expected_used.begin() + 0);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space at 8 and create a new free space there
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... | XXX | XXX | XXX | ... - ... | XXX | ... | XXX |
    pos = BUFFER_SIZE - (NUM_BLOCKS - 8) * size;
    expected_free.at(1).link = pos;
    expected_free.emplace_back(pos, size, NO_LINK);
    expected_used.erase(expected_used.begin() + 4);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space starting at 7 and join 5-6-7-8
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... | XXX | XXX | XXX | ... - ... - ... - ... | XXX |
    pos = BUFFER_SIZE - (NUM_BLOCKS - 7) * size;
    expected_free.at(1).link = expected_free.at(2).link;
    expected_free.at(1).size += size + expected_free.at(2).size;
    expected_free.erase(expected_free.begin() + 2);
    expected_used.erase(expected_used.begin() + 3);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space starting at 9 and expand size of free space at 5
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... | XXX | XXX | XXX | ... - ... - ... - ... - ... |
    pos = BUFFER_SIZE - (NUM_BLOCKS - 9) * size;
    expected_free.at(1).size += size;
    expected_used.erase(expected_used.begin() + 3);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space at 3 and create a new free space there
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... | XXX | ... | XXX | ... - ... - ... - ... - ... |
    pos = BUFFER_SIZE - (NUM_BLOCKS - 3) * size;
    expected_free.at(0).link = pos;
    expected_free.emplace(expected_free.begin() + 1, pos, size, expected_free.at(1).location);
    expected_used.erase(expected_used.begin() + 1);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space at 2 and coalesce first and second free spaces
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... - ... - ... | XXX | ... - ... - ... - ... - ... |
    pos = BUFFER_SIZE - (NUM_BLOCKS - 2) * size;
    expected_free.at(0).link = expected_free.at(1).link;
    expected_free.at(0).size += size + expected_free.at(1).size;
    expected_free.erase(expected_free.begin() + 1);
    expected_used.erase(expected_used.begin() + 0);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space at 4 and coalesce first and second free spaces
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... - ... - ... - ... - ... - ... - ... - ... - ... |
    pos = BUFFER_SIZE - (NUM_BLOCKS - 4) * size;
    expected_free.at(0).link = expected_free.at(1).link;
    expected_free.at(0).size += size + expected_free.at(1).size;
    expected_free.erase(expected_free.begin() + 1);
    expected_used.erase(expected_used.begin() + 0);
    CheckDeallocate(pos, size, expected_free, expected_used);

    EXPECT_EQ(FreeSegments.size(), 1);
    EXPECT_TRUE(UsedSegments.empty());
}

}  // namespace memory
