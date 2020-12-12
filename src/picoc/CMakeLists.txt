set(EXE_TARGET_NAME picoc)

# Sources --------------------------------------------------------------------
set(${EXE_TARGET_NAME}_SRC
        src/picoc_clibrary.c
        src/picoc_debug.c
        src/picoc_expression.c
        src/picoc_heap.c
        src/picoc_include.c
        src/picoc_lex.c
        src/picoc_parse.c
        src/picoc_picoc.c
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
    set(${EXE_TARGET_NAME}_SRC
            ${${EXE_TARGET_NAME}_SRC}
            src/cstdlib/picoc_unistd.c
            src/library_unix.c
            src/platform_unix.c
            )
    add_definitions( -DUNIX_HOST )
elseif(MSVC)
    message(STATUS "Building for Windows host")
    set(${EXE_TARGET_NAME}_SRC
            ${${EXE_TARGET_NAME}_SRC}
            src/library_msvc.c
            src/platform_msvc.c
            )
else()
    message(STATUS "No platform set")
endif()

set(${EXE_TARGET_NAME}_HDR_PUBLIC
        include/picoc/picoc_interpreter.h
        include/picoc/picoc_license.h
        include/picoc/picoc_picoc.h
        include/picoc/picoc_platform.h
        include/picoc/platform/platform_msvc.h
        include/picoc/platform/platform_unix.h
        )

configure_file(Config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/picoc/config.h)

# Dependencies --------------------------------------------------------------
include(generate_product_version)
generate_product_version(
        VersionFilesOutputVariable
        COMPANY_NAME "Pyramid Technologies, Inc"
        NAME "picoc (C) 2020"
        VERSION_MAJOR ${PROJECT_VERSION_MAJOR}
        VERSION_MINOR ${PROJECT_VERSION_MINOR}
        VERSION_PATCH ${PROJECT_VERSION_PATCH}
        VERSION_REVISION ${PROJECT_VERSION_TWEAK}
)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR})

set(${EXE_TARGET_NAME}_LIBS)
if(UNIX)
    set(${EXE_TARGET_NAME}_LIBS
            ${${EXE_TARGET_NAME}_LIBS}
            readline
            m)
endif(UNIX)

# Binary --------------------------------------------------------------------
add_executable(${EXE_TARGET_NAME}
        ${${EXE_TARGET_NAME}_SRC}
        ${${EXE_TARGET_NAME}_HDR_PUBLIC}
        ${VersionFilesOutputVariable})

target_link_libraries(${EXE_TARGET_NAME} ${${EXE_TARGET_NAME}_LIBS})

set_target_properties(${EXE_TARGET_NAME} PROPERTIES
        VERSION ${${PROJECT_NAME}_VERSION})

# Installation---------------------------------------------------------------
install(TARGETS ${EXE_TARGET_NAME} DESTINATION bin)
install(DIRECTORY src/ DESTINATION src)

message(STATUS "Created target ${EXE_TARGET_NAME} for export ${PROJECT_NAME} v${PROJECT_VERSION}.")

