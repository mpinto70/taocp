set(memory_srcs
    linear_test.cpp
    memory_test_utils.cpp
)

set(memory_libs
    memory
)

add_unit_test(
    memory
    memory_srcs
    memory_libs
)
