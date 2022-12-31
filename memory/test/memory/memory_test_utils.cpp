#include "memory_test_utils.h"

#include <iomanip>
#include <iostream>
#include <random>

namespace memory {
SizeType RandomValue(SizeType min, SizeType max) {
    static std::mt19937 gen((std::random_device())());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

void PrintHeader(const std::string& name) {
    std::cout << "++++++++++ " << name << " ++++++++++\n";
}

void PrintChar(unsigned char c) {
    if (std::isprint(c)) {
        std::cout << c;
    } else {
        std::cout << '?';
    }
}

void PrintMemory(const MemoryT& memory, const std::string& name) {
    PrintHeader(name);
    std::cout << std::setw(8) << memory.size() << " [ ";
    for (const auto byte : memory) {
        PrintChar(ToInteger<unsigned char>(byte));
    }
    std::cout << " ]\n";
}
}  // namespace memory
