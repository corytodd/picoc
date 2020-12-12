set(TEST_MODULE UnitTests)

configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif()

if (MSVC)
    # Disable warning for iota usage and unknown pragma warnings
    add_compile_options("/wd4068")
endif(MSVC)

set(GEN_FILE GeneratedTests.hpp)
ADD_CUSTOM_COMMAND(
        OUTPUT GeneratedTests.hpp
        COMMAND generator ${GEN_FILE}
        DEPENDS generator
        COMMENT "Generating Tiger Grammar"
)

add_executable(
        generator
        src/generator.cpp
)

add_executable(
        ${TEST_MODULE}
        src/main.cpp
        src/runner.cpp
        ${GEN_FILE}
)

target_link_libraries(
        ${TEST_MODULE} PRIVATE
        picoc::picoc
        gtest_main
)

add_custom_target(copy-files ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/test_data
        ${CMAKE_CURRENT_BINARY_DIR}/test_data
        )
add_dependencies(${TEST_MODULE} copy-files)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include(CTest)
enable_testing()

add_test(NAME all_tests COMMAND ${TEST_MODULE})

set(APP_HIDE_MODULES
        ${APP_HIDE_MODULES}
        Continuous
        Experimental
        googletest
        Nightly
        NightlyMemoryCheck
        )

set_target_properties(${TEST_MODULE} PROPERTIES FOLDER Tests)