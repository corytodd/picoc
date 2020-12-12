set(LIBRARY_TARGET_NAME picoc)

# Sources --------------------------------------------------------------------
set(${LIBRARY_TARGET_NAME}_SRC
        src/picoc_clibrary.c
        src/picoc_debug.c
        src/picoc_expression.c
        src/picoc_heap.c
        src/picoc_include.c
        src/picoc_lex.c
        src/picoc_parse.c
        src/picoc_platform.c
        src/picoc_table.c
        src/picoc_type.c
        src/picoc_variable.c
        src/cstdlib/picoc_ctype.c
        src/cstdlib/picoc_errno.c
        src/cstdlib/picoc_math.c
        src/cstdlib/picoc_stdbool.c
        src/cstdlib/picoc_stdio.c
        src/cstdlib/picoc_stdlib.c
        src/cstdlib/picoc_string.c
        src/cstdlib/picoc_time.c
        )


if(UNIX)
    message(STATUS "Building for Unix host")
    set(${LIBRARY_TARGET_NAME}_SRC
            ${${LIBRARY_TARGET_NAME}_SRC}
            src/cstdlib/picoc_unistd.c
            src/platform/library_unix.c
            src/platform/platform_unix.c
            )
elseif(MSVC)
    message(STATUS "Building for Windows host")
    set(${LIBRARY_TARGET_NAME}_SRC
            ${${LIBRARY_TARGET_NAME}_SRC}
            src/platform/library_msvc.c
            src/platform/platform_msvc.c
            )
else()
    message(STATUS "No platform set")
endif()

set(${LIBRARY_TARGET_NAME}_HDR_PUBLIC
        include/picoc/picoc_interpreter.h
        include/picoc/picoc_license.h
        include/picoc/picoc_picoc.h
        include/picoc/picoc_platform.h
        include/picoc/platform/platform_msvc.h
        include/picoc/platform/platform_unix.h
        )

configure_file(Config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/picoc/config.h)
add_definitions( -DPICOC_VERSION="${PROJECT_VERSION}")

# Dependencies --------------------------------------------------------------
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR})

set(${LIBRARY_TARGET_NAME}_LIBS)
if(UNIX)
    set(${LIBRARY_TARGET_NAME}_LIBS
            ${${LIBRARY_TARGET_NAME}_LIBS}
            readline
            m)
endif(UNIX)

# Binary --------------------------------------------------------------------
add_library(${LIBRARY_TARGET_NAME}
        ${${LIBRARY_TARGET_NAME}_SRC}
        ${${LIBRARY_TARGET_NAME}_HDR_PUBLIC})

add_library(${PROJECT_NAME}::${LIBRARY_TARGET_NAME} ALIAS ${LIBRARY_TARGET_NAME})

target_link_libraries(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_LIBS})

set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES
        VERSION ${${PROJECT_NAME}_VERSION})

# Exports --------------------------------------------------------------------
set(EXPORTED_HEADERS ${${LIBRARY_TARGET_NAME}_HDR_PUBLIC})

set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES
        VERSION ${${PROJECT_NAME}_VERSION}
        PUBLIC_HEADER "${EXPORTED_HEADERS}")

target_include_directories(${LIBRARY_TARGET_NAME} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>")

target_include_directories(${LIBRARY_TARGET_NAME} PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>")

# Installation---------------------------------------------------------------
install(TARGETS ${LIBRARY_TARGET_NAME} DESTINATION bin)
install(DIRECTORY src/ DESTINATION src)

message(STATUS "Created target ${LIBRARY_TARGET_NAME} for export ${PROJECT_NAME} v${PROJECT_VERSION}.")

