################################################################################
# Google Test Setup - BEGIN ####################################################
################################################################################

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.11.0
)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

################################################################################
# Google Test Setup - END ######################################################
################################################################################

function(add_unit_test test_name sources_var libs_var)
    set(unit_test_name unit_${test_name})

    add_executable(
        ${unit_test_name}
        ${${sources_var}}
    )

    set_target_properties(
        ${unit_test_name}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test/bin
    )

    target_link_libraries(
        ${unit_test_name}
        PRIVATE ${${libs_var}}
        PRIVATE gtest gmock_main
    )

    add_test(NAME ${unit_test_name} COMMAND ${unit_test_name})
endfunction(add_unit_test)

function(_add_lib lib_name sources_var directory)
    add_library(
        ${lib_name}
        ${${sources_var}}
    )

    target_include_directories(
        ${lib_name}
        PRIVATE ${gtest_SOURCE_DIR}/include
        PRIVATE ${gmock_SOURCE_DIR}/include
    )

    set_target_properties(
        ${lib_name}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${directory}/lib
    )
endfunction(_add_lib)

function(add_test_lib test_lib_name sources_var)
    _add_lib(
        ${test_lib_name}
        ${sources_var}
        test
    )
endfunction(add_test_lib)

function(add_mock_lib mck_lib_name sources_var)
    _add_lib(
        ${mck_lib_name}
        ${sources_var}
        mck
    )
endfunction(add_mock_lib)
