#pragma once

#include "memory/memory_utils.h"

#include <string>

namespace memory {

size_type random_value(size_type min, size_type max);

void print_header(const std::string& name);

void print_char(unsigned char c);

void print_memory(const MemoryT& memory, const std::string& name);

}  // namespace memory