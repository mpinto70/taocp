#include "memory_test_utils.h"

#include <iomanip>
#include <iostream>
#include <random>

namespace memory {
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

void print_memory(const MemoryT& memory, const std::string& name) {
    print_header(name);
    std::cout << std::setw(8) << memory.size() << " [ ";
    for (const auto byte : memory) {
        print_char(ToInteger<unsigned char>(byte));
    }
    std::cout << " ]\n";
}
}  // namespace memory
