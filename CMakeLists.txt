cmake_minimum_required(VERSION 3.15.2)

project(picoc
        LANGUAGES C
        # major.minor.patch/revision.tweak
        VERSION 4.0.0.0)

include(GNUInstallDirs)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Windows compat -------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

# Options -------------------------------------------------------------------------
option(BUILD_TESTING "Create tests using CMake" OFF)

# Cross compat --------------------------------------------------------------------
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)

# Application --------------------------------------------------------------------
add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()