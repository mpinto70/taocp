#include "memory_test_utils.h"

#include "memory/linear.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <random>

namespace memory::linear {

extern MemoryT Memory;
extern Segments FreeSegments;
extern Segments UsedSegments;

std::ostream& operator<<(std::ostream& out, const Segment& segment) {
    out << segment.location << " / " << segment.size;

    return out;
}

bool operator==(const Segment& lhs, const Segment& rhs) {
    return lhs.location == rhs.location && lhs.size == rhs.size;
}

bool operator!=(const Segment& lhs, const Segment& rhs) {
    return !(lhs == rhs);
}

void PrintSegment(const Segment& segment, char s) {
    std::cout << s << " - ";
    std::cout << std::setw(4) << segment.location << " / ";
    std::cout << std::setw(4) << segment.size << " [ ";
    for (SizeType i = 0; i < segment.size; ++i) {
        SizeType idx = segment.location + i;
        const auto c = std::to_integer<unsigned char>(Memory[idx]);
        PrintChar(c);
    }
    std::cout << " ]\n";
}

void PrintSegments(const std::string& name) {
    PrintHeader(name);
    SizeType free_idx = 0;
    SizeType used_idx = 0;
    while (free_idx < FreeSegments.size() || used_idx < UsedSegments.size()) {
        const Segment* free_seg =
                free_idx < FreeSegments.size() ? &FreeSegments.at(free_idx) : nullptr;
        const Segment* used_seg =
                used_idx < UsedSegments.size() ? &UsedSegments.at(used_idx) : nullptr;
        if (free_seg != nullptr && used_seg != nullptr) {
            if (free_seg->location <= used_seg->location) {
                PrintSegment(*free_seg, 'f');
                ++free_idx;
            } else {
                PrintSegment(*used_seg, 'u');
                ++used_idx;
            }
        } else if (free_seg != nullptr) {
            PrintSegment(*free_seg, 'f');
            ++free_idx;
        } else {
            PrintSegment(*used_seg, 'u');
            ++used_idx;
        }
    }
}

void PrintSegments(const std::string& name, const Segments& segments) {
    PrintHeader(name);
    for (std::size_t i = 0; i < segments.size(); ++i) {
        const auto& segment = segments.at(i);
        std::cout << std::setw(3) << i << " - ";
        PrintSegment(segment, 'X');
    }
}

class MemoryTest : public ::testing::Test {
public:
    ~MemoryTest() override { Deinit(); }
};

TEST_F(MemoryTest, SegmentConstruction) {
    constexpr Segment seg;

    EXPECT_EQ(seg.location, 0);
    EXPECT_EQ(seg.size, 0);
}

void CheckInit(int line, SizeType memory_size) {
    SCOPED_TRACE("From line " + std::to_string(line));
    // put garbage
    FreeSegments.resize(30, Segment{});
    UsedSegments.resize(35, Segment{});
    Memory.resize(50, std::byte{ 'c' });

    // test init state
    Init(memory_size);
    EXPECT_EQ(FreeSegments.size(), 1);
    const auto& segment = FreeSegments.at(0);
    EXPECT_EQ(segment.location, 0);
    EXPECT_EQ(segment.size, memory_size);
    EXPECT_EQ(Memory.size(), memory_size);
    EXPECT_TRUE(UsedSegments.empty());
}

TEST_F(MemoryTest, InitState) {
    CheckInit(__LINE__, 457);
    CheckInit(__LINE__, 300'000);
}

void CheckAllocate(
        int line,
        SizeType location,
        SizeType size,
        const Segments& expected_free,
        const Segments& expected_used) {
    SCOPED_TRACE("For location " + std::to_string(location) + " and size " + std::to_string(size));
    SCOPED_TRACE("From line " + std::to_string(line));
    EXPECT_NE(AllocateFilled(size, 'x'), nullptr);

    // PrintSegments("allocated " + std::to_string(location));

    EXPECT_EQ(FreeSegments, expected_free);
    EXPECT_EQ(UsedSegments, expected_used);
}

TEST_F(MemoryTest, Allocate) {
    constexpr SizeType memory_size = 537;
    Init(memory_size);
    Segments expected_free = { Segment{ 0, memory_size } };
    Segments expected_used;
    SizeType location = memory_size;
    while (FreeSegments.at(0).size > 40) {
        const SizeType size = RandomValue(30, 40);
        location -= size;
        expected_free.at(0).size -= size;
        expected_used.emplace(expected_used.begin(), location, size);
        CheckAllocate(__LINE__, location, size, expected_free, expected_used);
    }

    EXPECT_FALSE(UsedSegments.empty());
}

void CheckDeallocate(
        int line,
        SizeType location,
        SizeType size,
        const Segments& expected_free,
        const Segments& expected_used) {
    SCOPED_TRACE("For location " + std::to_string(location) + " and size " + std::to_string(size));
    SCOPED_TRACE("From line " + std::to_string(line));
    std::fill_n(Memory.begin() + location, size, std::byte{ '.' });
    EXPECT_TRUE(Deallocate(&Memory[location]));

    // PrintSegments("deallocated " + std::to_string(location));

    EXPECT_EQ(FreeSegments, expected_free);
    EXPECT_EQ(UsedSegments, expected_used);
}

TEST_F(MemoryTest, Deallocate) {
    constexpr SizeType memory_size = 357;
    Init(memory_size);
    constexpr SizeType NUM_BLOCKS = 10;
    constexpr SizeType size = memory_size / NUM_BLOCKS;
    SizeType allocated = 0;
    while (true) {
        auto ptr = reinterpret_cast<unsigned char*>(AllocateFilled(size, 'x'));
        if (ptr == nullptr) {
            break;
        }
        allocated += size;
    }
    EXPECT_EQ(allocated, NUM_BLOCKS * size);

    // PrintSegments("filled");

    auto expected_free = FreeSegments;
    auto expected_used = UsedSegments;
    SizeType pos = 0;

    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX |

    // remove used space at 6 and create a new free space there
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | XXX | XXX | XXX | XXX | XXX | XXX | ... | XXX | XXX | XXX |
    pos = memory_size - (NUM_BLOCKS - 6) * size;
    expected_free.emplace_back(pos, size);
    expected_used.erase(expected_used.begin() + 6);
    CheckDeallocate(__LINE__, pos, size, expected_free, expected_used);

    // remove used space at 5 and coalesce 5-6
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | XXX | XXX | XXX | XXX | XXX | ... - ... | XXX | XXX | XXX |
    pos = memory_size - (NUM_BLOCKS - 5) * size;
    expected_free.at(1).location = pos;
    expected_free.at(1).size += size;
    expected_used.erase(expected_used.begin() + 5);
    CheckDeallocate(__LINE__, pos, size, expected_free, expected_used);

    // remove used space at 0 and expand size of first free space
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... | XXX | XXX | XXX | XXX | ... - ... | XXX | XXX | XXX |
    pos = memory_size - (NUM_BLOCKS - 0) * size;
    expected_free.at(0).size += size;
    expected_used.erase(expected_used.begin() + 0);
    CheckDeallocate(__LINE__, pos, size, expected_free, expected_used);

    // remove used space at 1 and expand size of first free space
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... | XXX | XXX | XXX | ... - ... | XXX | XXX | XXX |
    pos = memory_size - (NUM_BLOCKS - 1) * size;
    expected_free.at(0).size += size;
    expected_used.erase(expected_used.begin() + 0);
    CheckDeallocate(__LINE__, pos, size, expected_free, expected_used);

    // remove used space at 8 and create a new free space there
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... | XXX | XXX | XXX | ... - ... | XXX | ... | XXX |
    pos = memory_size - (NUM_BLOCKS - 8) * size;
    expected_free.emplace_back(pos, size);
    expected_used.erase(expected_used.begin() + 4);
    CheckDeallocate(__LINE__, pos, size, expected_free, expected_used);

    // remove used space starting at 7 and join 5-6-7-8
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... | XXX | XXX | XXX | ... - ... - ... - ... | XXX |
    pos = memory_size - (NUM_BLOCKS - 7) * size;
    expected_free.at(1).size += size + expected_free.at(2).size;
    expected_free.erase(expected_free.begin() + 2);
    expected_used.erase(expected_used.begin() + 3);
    CheckDeallocate(__LINE__, pos, size, expected_free, expected_used);

    // remove used space starting at 9 and expand size of free space at 5
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... | XXX | XXX | XXX | ... - ... - ... - ... - ... |
    pos = memory_size - (NUM_BLOCKS - 9) * size;
    expected_free.at(1).size += size;
    expected_used.erase(expected_used.begin() + 3);
    CheckDeallocate(__LINE__, pos, size, expected_free, expected_used);

    // remove used space at 3 and create a new free space there
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... | XXX | ... | XXX | ... - ... - ... - ... - ... |
    pos = memory_size - (NUM_BLOCKS - 3) * size;
    expected_free.emplace(expected_free.begin() + 1, pos, size);
    expected_used.erase(expected_used.begin() + 1);
    CheckDeallocate(__LINE__, pos, size, expected_free, expected_used);

    // remove used space at 2 and coalesce first and second free spaces
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... - ... - ... | XXX | ... - ... - ... - ... - ... |
    pos = memory_size - (NUM_BLOCKS - 2) * size;
    expected_free.at(0).size += size + expected_free.at(1).size;
    expected_free.erase(expected_free.begin() + 1);
    expected_used.erase(expected_used.begin() + 0);
    CheckDeallocate(__LINE__, pos, size, expected_free, expected_used);

    // remove used space at 4 and coalesce first and second free spaces
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... - ... - ... - ... - ... - ... - ... - ... - ... |
    pos = memory_size - (NUM_BLOCKS - 4) * size;
    expected_free.at(0).size += size + expected_free.at(1).size;
    expected_free.erase(expected_free.begin() + 1);
    expected_used.erase(expected_used.begin() + 0);
    CheckDeallocate(__LINE__, pos, size, expected_free, expected_used);

    EXPECT_EQ(FreeSegments.size(), 1);
    EXPECT_TRUE(UsedSegments.empty());
}

TEST_F(MemoryTest, AllocateDeallocate) {
    constexpr SizeType memory_size = 239;
    Init(memory_size);
    constexpr SizeType NUM_BLOCKS = 10;
    constexpr SizeType size = memory_size / NUM_BLOCKS;
    while (true) {
        auto ptr = reinterpret_cast<unsigned char*>(AllocateFilled(size, 'x'));
        if (ptr == nullptr) {
            break;
        }
    }

    // PrintSegments("filled");

    auto expected_free = FreeSegments;
    auto expected_used = UsedSegments;
    SizeType pos = 0;

    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX |

    // remove used space at 6 and create a new free space there
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | XXX | XXX | XXX | XXX | XXX | XXX | ... | XXX | XXX | XXX |
    pos = memory_size - (NUM_BLOCKS - 6) * size;
    expected_free.emplace_back(pos, size);
    expected_used.erase(expected_used.begin() + 6);
    CheckDeallocate(__LINE__, pos, size, expected_free, expected_used);

    // remove used space at 5 and coalesce 5-6
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | XXX | XXX | XXX | XXX | XXX | ... - ... | XXX | XXX | XXX |
    pos = memory_size - (NUM_BLOCKS - 5) * size;
    expected_free.at(1).location = pos;
    expected_free.at(1).size += size;
    expected_used.erase(expected_used.begin() + 5);
    CheckDeallocate(__LINE__, pos, size, expected_free, expected_used);

    // allocate size that will occupy 6
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | XXX | XXX | XXX | XXX | XXX | ... | XXX | XXX | XXX | XXX |
    pos = memory_size - (NUM_BLOCKS - 6) * size;
    expected_free.at(1).size -= size;
    expected_used.emplace(expected_used.begin() + 5, pos, size);
    CheckAllocate(__LINE__, pos, size, expected_free, expected_used);

    // allocate size that will occupy 5 and remove free space
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX |
    pos = memory_size - (NUM_BLOCKS - 5) * size;
    expected_free.erase(expected_free.begin() + 1);
    expected_used.emplace(expected_used.begin() + 5, pos, size);
    CheckAllocate(__LINE__, pos, size, expected_free, expected_used);
}

void CheckSegmentsAreSorted(int line, const Segments& segments, bool is_used) {
    SCOPED_TRACE("From line " + std::to_string(line));

    for (std::size_t i = 1; i < segments.size(); ++i) {
        const auto& current = segments.at(i);
        const auto& previous = segments.at(i - 1);
        SCOPED_TRACE(i);
        SCOPED_TRACE(current);
        SCOPED_TRACE(previous);
        if (is_used) {
            EXPECT_GE(current.location, previous.location + previous.size);
        } else {
            EXPECT_GT(current.location, previous.location + previous.size);
        }
    }
}

void CheckSegmentLocation(
        int line,
        std::size_t& idx,
        SizeType& expected_location,
        const Segment& segment) {
    SCOPED_TRACE("From line " + std::to_string(line));
    EXPECT_EQ(segment.location, expected_location) << idx << " for " << segment;
    expected_location = segment.location + segment.size;
    ++idx;
}

void CheckSegmentsAreCongruent(int line) {
    SCOPED_TRACE("From line " + std::to_string(line));

    std::size_t idx_free = 0, idx_used = 0;
    SizeType expected_location = 0;
    SizeType expected_free_location = kInvalidSize;
    while (idx_free < FreeSegments.size() || idx_used < UsedSegments.size()) {
        Segment* free = idx_free < FreeSegments.size() ? &FreeSegments.at(idx_free) : nullptr;
        Segment* used = idx_used < UsedSegments.size() ? &UsedSegments.at(idx_used) : nullptr;
        if ((free != nullptr && used != nullptr && free->location <= used->location)
            || used == nullptr) {
            CheckSegmentLocation(__LINE__, idx_free, expected_location, *free);
            if (expected_free_location != kInvalidSize) {
                // check that there are no two consecutive free spaces
                EXPECT_GT(free->location, expected_free_location) << idx_free << " for " << *free;
            }
            expected_free_location = free->location + free->size;
        } else {
            CheckSegmentLocation(__LINE__, idx_used, expected_location, *used);
        }
    }
    CheckSegmentsAreSorted(line, FreeSegments, false);
    CheckSegmentsAreSorted(line, UsedSegments, true);
}

TEST_F(MemoryTest, RandomWork) {
    constexpr SizeType memory_size = 300;
    constexpr SizeType min_size = memory_size / 15;
    constexpr SizeType max_size = memory_size / 10;
    Init(memory_size);
    while (FreeSegments.at(0).size > memory_size / 2) {
        AllocateFilled(RandomValue(min_size, max_size), 'x');
    }
    CheckSegmentsAreCongruent(__LINE__);

    // PrintMemory(Memory, "allocated");
    // PrintSegments("initial");

    for (std::size_t i = 0; i < 20; ++i) {
        const bool is_allocate = RandomValue(0, 2) > 0;
        if (is_allocate) {
            const auto size = RandomValue(min_size, max_size);
            if (AllocateFilled(size, 'x') != nullptr) {
                // PrintSegments("allocated");
                // PrintMemory(Memory, "allocated " + std::to_string(size));
            }
        } else {
            if (not UsedSegments.empty()) {
                const auto idx = RandomValue(1, UsedSegments.size()) - 1;
                const auto segment = UsedSegments.at(idx);
                std::fill_n(Memory.begin() + segment.location, segment.size, std::byte{ '.' });
                EXPECT_TRUE(Deallocate(&Memory.at(segment.location)));
                // PrintSegments("deallocated");
                // PrintMemory(
                //         Memory,
                //         "deallocated " + std::to_string(idx) + " - "
                //                 + std::to_string(segment.location) + " - "
                //                 + std::to_string(segment.size));
            }
        }
        CheckSegmentsAreCongruent(__LINE__);
    }

    // PrintSegments("random");
}

}  // namespace memory::linear
