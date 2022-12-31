#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace memory {

using SizeType = uint64_t;
constexpr SizeType kInvalidSize = std::numeric_limits<SizeType>::max();

using Byte = std::byte;
using MemoryT = std::vector<Byte>;

template <typename T>
constexpr T (*ToInteger)(Byte) = &std::to_integer<T>;

}  // namespace memory
