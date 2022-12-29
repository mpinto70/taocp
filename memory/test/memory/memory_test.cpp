#include "memory/memory.h"

#include <gtest/gtest.h>

#include <boost/filesystem.hpp>

namespace memory {

TEST(MemoryTest, square) {
    EXPECT_EQ(square(2), 4);
    EXPECT_EQ(square(3), 9);
}

}  // namespace memory
