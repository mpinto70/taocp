#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace memory {

using size_type = uint64_t;
constexpr size_type INVALID_SIZE = std::numeric_limits<size_type>::max();

using Byte = std::byte;
using MemoryT = std::vector<Byte>;

template <typename T>
constexpr T (*ToInteger)(Byte) = &std::to_integer<T>;

}  // namespace memory
