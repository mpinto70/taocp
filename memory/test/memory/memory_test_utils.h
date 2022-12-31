#pragma once

#include "memory/memory_utils.h"

#include <string>

namespace memory {

SizeType RandomValue(SizeType min, SizeType max);

void PrintHeader(const std::string& name);

void PrintChar(unsigned char c);

void PrintMemory(const MemoryT& memory, const std::string& name);

}  // namespace memory