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

add_executable(
        ${TEST_MODULE}
        src/main.cpp
)

target_link_libraries(
        ${TEST_MODULE}
        gtest_main
)

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