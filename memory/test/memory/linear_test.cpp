#include "memory/linear.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <random>

namespace memory {

extern std::vector<std::byte> Memory;
extern Segments FreeSegments;
extern Segments UsedSegments;

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

void print_header(const std::string& name) {
    std::cout << "++++++++++ " << name << " ++++++++++\n";
}

void print_char(unsigned char c) {
    if (std::isprint(c)) {
        std::cout << c;
    } else {
        std::cout << '?';
    }
}

void print_memory(const std::string& name) {
    print_header(name);
    std::cout << std::setw(8) << Memory.size() << " [ ";
    for (const auto byte : Memory) {
        print_char(std::to_integer<unsigned char>(byte));
    }
    std::cout << " ]\n";
}

void print_segment(const Segment& segment, char s) {
    std::cout << s << " - ";
    std::cout << std::setw(4) << segment.location << " / ";
    std::cout << std::setw(4) << segment.size << " / ";
    if (segment.link != NO_LINK) {
        std::cout << std::setw(4) << segment.link;
    } else {
        std::cout << " ---";
    }
    std::cout << " [ ";
    for (size_type i = 0; i < segment.size; ++i) {
        size_type idx = segment.location + i;
        const auto c = std::to_integer<unsigned char>(Memory[idx]);
        print_char(c);
    }
    std::cout << " ]\n";
}

void print_segments(const std::string& name) {
    print_header(name);
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

void print_segments(const std::string& name, const Segments& segments) {
    print_header(name);
    for (std::size_t i = 0; i < segments.size(); ++i) {
        const auto& segment = segments.at(i);
        std::cout << std::setw(3) << i << " - ";
        print_segment(segment, 'X');
    }
}

class MemoryTest : public ::testing::Test {
public:
    ~MemoryTest() override { deinit(); }
};

TEST_F(MemoryTest, SegmentConstruction) {
    constexpr Segment seg;

    EXPECT_EQ(seg.location, 0);
    EXPECT_EQ(seg.size, 0);
    EXPECT_EQ(seg.link, NO_LINK);
}

void CheckInit(size_type memory_size) {
    // put garbage
    FreeSegments.resize(30, Segment{});
    UsedSegments.resize(35, Segment{});
    Memory.resize(50, std::byte{ 'c' });

    // test init state
    SCOPED_TRACE("For memory size = " + std::to_string(memory_size));
    init(memory_size);
    EXPECT_EQ(FreeSegments.size(), 1);
    const auto& segment = FreeSegments.at(0);
    EXPECT_EQ(segment.location, 0);
    EXPECT_EQ(segment.size, memory_size);
    EXPECT_EQ(segment.link, NO_LINK);
    EXPECT_EQ(Memory.size(), memory_size);
    EXPECT_TRUE(UsedSegments.empty());
}

TEST_F(MemoryTest, InitState) {
    CheckInit(457);
    CheckInit(300'000);
}

void CheckAllocate(
        size_type location,
        size_type size,
        const Segments& expected_free,
        const Segments& expected_used) {
    SCOPED_TRACE(location);
    EXPECT_NE(allocate_filled(size, 'x'), nullptr);

    // print_segments("allocated " + std::to_string(location));

    EXPECT_EQ(FreeSegments, expected_free);
    EXPECT_EQ(UsedSegments, expected_used);
}

TEST_F(MemoryTest, Allocate) {
    constexpr size_type memory_size = 537;
    init(memory_size);
    Segments expected_free = { Segment{ 0, memory_size, NO_LINK } };
    Segments expected_used;
    size_type location = memory_size;
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
        const Segments& expected_free,
        const Segments& expected_used) {
    SCOPED_TRACE(location);
    std::fill_n(Memory.begin() + location, size, std::byte{ '.' });
    EXPECT_TRUE(deallocate(&Memory[location]));

    // print_segments("deallocated " + std::to_string(location));

    EXPECT_EQ(FreeSegments, expected_free);
    EXPECT_EQ(UsedSegments, expected_used);
}

TEST_F(MemoryTest, Deallocate) {
    constexpr size_type memory_size = 357;
    init(memory_size);
    constexpr size_type NUM_BLOCKS = 10;
    constexpr size_type size = memory_size / NUM_BLOCKS;
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
    pos = memory_size - (NUM_BLOCKS - 6) * size;
    expected_free.at(0).link = pos;
    expected_free.emplace_back(pos, size, NO_LINK);
    expected_used.erase(expected_used.begin() + 6);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space at 5 and coalesce 5-6
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | XXX | XXX | XXX | XXX | XXX | ... - ... | XXX | XXX | XXX |
    pos = memory_size - (NUM_BLOCKS - 5) * size;
    expected_free.at(0).link = pos;
    expected_free.at(1).location = pos;
    expected_free.at(1).size += size;
    expected_used.erase(expected_used.begin() + 5);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space at 0 and expand size of first free space
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... | XXX | XXX | XXX | XXX | ... - ... | XXX | XXX | XXX |
    pos = memory_size - (NUM_BLOCKS - 0) * size;
    expected_free.at(0).size += size;
    expected_used.erase(expected_used.begin() + 0);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space at 1 and expand size of first free space
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... | XXX | XXX | XXX | ... - ... | XXX | XXX | XXX |
    pos = memory_size - (NUM_BLOCKS - 1) * size;
    expected_free.at(0).size += size;
    expected_used.erase(expected_used.begin() + 0);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space at 8 and create a new free space there
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... | XXX | XXX | XXX | ... - ... | XXX | ... | XXX |
    pos = memory_size - (NUM_BLOCKS - 8) * size;
    expected_free.at(1).link = pos;
    expected_free.emplace_back(pos, size, NO_LINK);
    expected_used.erase(expected_used.begin() + 4);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space starting at 7 and join 5-6-7-8
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... | XXX | XXX | XXX | ... - ... - ... - ... | XXX |
    pos = memory_size - (NUM_BLOCKS - 7) * size;
    expected_free.at(1).link = expected_free.at(2).link;
    expected_free.at(1).size += size + expected_free.at(2).size;
    expected_free.erase(expected_free.begin() + 2);
    expected_used.erase(expected_used.begin() + 3);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space starting at 9 and expand size of free space at 5
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... | XXX | XXX | XXX | ... - ... - ... - ... - ... |
    pos = memory_size - (NUM_BLOCKS - 9) * size;
    expected_free.at(1).size += size;
    expected_used.erase(expected_used.begin() + 3);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space at 3 and create a new free space there
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... | XXX | ... | XXX | ... - ... - ... - ... - ... |
    pos = memory_size - (NUM_BLOCKS - 3) * size;
    expected_free.at(0).link = pos;
    expected_free.emplace(expected_free.begin() + 1, pos, size, expected_free.at(1).location);
    expected_used.erase(expected_used.begin() + 1);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space at 2 and coalesce first and second free spaces
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... - ... - ... | XXX | ... - ... - ... - ... - ... |
    pos = memory_size - (NUM_BLOCKS - 2) * size;
    expected_free.at(0).link = expected_free.at(1).link;
    expected_free.at(0).size += size + expected_free.at(1).size;
    expected_free.erase(expected_free.begin() + 1);
    expected_used.erase(expected_used.begin() + 0);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space at 4 and coalesce first and second free spaces
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | ... - ... - ... - ... - ... - ... - ... - ... - ... - ... |
    pos = memory_size - (NUM_BLOCKS - 4) * size;
    expected_free.at(0).link = expected_free.at(1).link;
    expected_free.at(0).size += size + expected_free.at(1).size;
    expected_free.erase(expected_free.begin() + 1);
    expected_used.erase(expected_used.begin() + 0);
    CheckDeallocate(pos, size, expected_free, expected_used);

    EXPECT_EQ(FreeSegments.size(), 1);
    EXPECT_TRUE(UsedSegments.empty());
}

TEST_F(MemoryTest, AllocateDeallocate) {
    constexpr size_type memory_size = 239;
    init(memory_size);
    constexpr size_type NUM_BLOCKS = 10;
    constexpr size_type size = memory_size / NUM_BLOCKS;
    while (true) {
        auto ptr = reinterpret_cast<unsigned char*>(allocate_filled(size, 'x'));
        if (ptr == nullptr) {
            break;
        }
    }

    // print_segments("filled");

    auto expected_free = FreeSegments;
    auto expected_used = UsedSegments;
    size_type pos = 0;

    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX |

    // remove used space at 6 and create a new free space there
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | XXX | XXX | XXX | XXX | XXX | XXX | ... | XXX | XXX | XXX |
    pos = memory_size - (NUM_BLOCKS - 6) * size;
    expected_free.at(0).link = pos;
    expected_free.emplace_back(pos, size, NO_LINK);
    expected_used.erase(expected_used.begin() + 6);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // remove used space at 5 and coalesce 5-6
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | XXX | XXX | XXX | XXX | XXX | ... - ... | XXX | XXX | XXX |
    pos = memory_size - (NUM_BLOCKS - 5) * size;
    expected_free.at(0).link = pos;
    expected_free.at(1).location = pos;
    expected_free.at(1).size += size;
    expected_used.erase(expected_used.begin() + 5);
    CheckDeallocate(pos, size, expected_free, expected_used);

    // allocate size that will occupy 6
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | XXX | XXX | XXX | XXX | XXX | ... | XXX | XXX | XXX | XXX |
    pos = memory_size - (NUM_BLOCKS - 6) * size;
    expected_free.at(1).size -= size;
    expected_used.emplace(expected_used.begin() + 5, pos, size, NO_LINK);
    CheckAllocate(pos, size, expected_free, expected_used);

    // allocate size that will occupy 5 and remove free space
    // |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
    // | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX | XXX |
    pos = memory_size - (NUM_BLOCKS - 5) * size;
    expected_free.at(0).link = expected_free.at(1).link;
    expected_free.erase(expected_free.begin() + 1);
    expected_used.emplace(expected_used.begin() + 5, pos, size, NO_LINK);
    CheckAllocate(pos, size, expected_free, expected_used);
}

void CheckSegmentsAreSorted(const Segments& segments, bool is_used) {
    if (not segments.empty()) {
        EXPECT_EQ(segments.back().link, NO_LINK) << segments.back();
    }
    for (std::size_t i = 1; i < segments.size(); ++i) {
        const auto& current = segments.at(i);
        const auto& previous = segments.at(i - 1);
        SCOPED_TRACE(i);
        SCOPED_TRACE(current);
        SCOPED_TRACE(previous);
        if (is_used) {
            EXPECT_EQ(previous.link, NO_LINK);
            EXPECT_EQ(current.link, NO_LINK);
            EXPECT_GE(current.location, previous.location + previous.size);
        } else {
            EXPECT_NE(previous.link, NO_LINK);
            EXPECT_GT(current.location, previous.location + previous.size);
            EXPECT_EQ(current.location, previous.link);
        }
    }
}

void CheckSegmentsAreCongruent() {
    std::size_t idx_free = 0, idx_used = 0;
    size_type expected_free_location = 0;
    size_type expected_used_location = 0;
    while (idx_free < FreeSegments.size() || idx_used < UsedSegments.size()) {
        Segment* free = idx_free < FreeSegments.size() ? &FreeSegments.at(idx_free) : nullptr;
        Segment* used = idx_used < UsedSegments.size() ? &UsedSegments.at(idx_used) : nullptr;
        if (free != nullptr && used != nullptr) {
            if (free->location <= used->location) {
                EXPECT_EQ(free->location, expected_free_location) << idx_free << " for " << *free;
                expected_free_location = free->link;
                expected_used_location = free->location + free->size;
                ++idx_free;
            } else {
                EXPECT_EQ(used->location, expected_used_location) << idx_used << " for " << *used;
                expected_used_location = used->location + used->size;
                ++idx_used;
            }
        } else if (free != nullptr) {
            EXPECT_EQ(free->location, expected_free_location) << idx_free << " for " << *free;
            EXPECT_EQ(free->link, NO_LINK) << idx_free << " for " << *free;
            ++idx_free;
        } else {
            EXPECT_EQ(used->location, expected_used_location) << idx_used << " for " << *used;
            expected_used_location = used->location + used->size;
            ++idx_used;
        }
    }
    CheckSegmentsAreSorted(FreeSegments, false);
    CheckSegmentsAreSorted(UsedSegments, true);
}

TEST_F(MemoryTest, RandomWork) {
    constexpr size_type memory_size = 300;
    constexpr size_type min_size = memory_size / 15;
    constexpr size_type max_size = memory_size / 10;
    init(memory_size);
    while (FreeSegments.at(0).size > memory_size / 2) {
        allocate_filled(random_value(min_size, max_size), 'x');
    }
    CheckSegmentsAreCongruent();

    // print_memory("allocated");
    // print_segments("allocated");

    for (std::size_t i = 0; i < 2000; ++i) {
        const bool is_allocate = random_value(0, 2) > 0;
        if (is_allocate) {
            const auto size = random_value(min_size, max_size);
            if (allocate_filled(size, 'x') != nullptr) {
                // print_memory("allocated " + std::to_string(size));
            }
        } else {
            if (not UsedSegments.empty()) {
                const auto idx = random_value(1, UsedSegments.size()) - 1;
                const auto segment = UsedSegments.at(idx);
                std::fill_n(Memory.begin() + segment.location, segment.size, std::byte{ '.' });
                EXPECT_TRUE(deallocate(&Memory.at(segment.location)));
                // print_memory(
                //         "deallocated " + std::to_string(idx) + " - "
                //         + std::to_string(segment.location) + " - " + std::to_string(segment.size));
            }
        }
        CheckSegmentsAreCongruent();
    }

    // print_segments("random");
}

}  // namespace memory
