set(EXE_TARGET_NAME picocli)

set(${EXE_TARGET_NAME}_SRC
        src/main.c
        )

# Force VS to treat code as C++
SET_SOURCE_FILES_PROPERTIES( ${EXE_TARGET_NAME}_SRC PROPERTIES LANGUAGE CXX )

include(generate_product_version)
generate_product_version(
        VersionFilesOutputVariable
        COMPANY_NAME "X"
        NAME "picocli (C) 2020"
        VERSION_MAJOR ${PROJECT_VERSION_MAJOR}
        VERSION_MINOR ${PROJECT_VERSION_MINOR}
        VERSION_PATCH ${PROJECT_VERSION_PATCH}
        VERSION_REVISION ${PROJECT_VERSION_TWEAK}
)

add_executable(${EXE_TARGET_NAME} ${${EXE_TARGET_NAME}_SRC} ${VersionFilesOutputVariable})

target_link_libraries(${EXE_TARGET_NAME} PRIVATE
        picoc::picoc
        )

set_target_properties(${EXE_TARGET_NAME} PROPERTIES
        VERSION ${${PROJECT_NAME}_VERSION})

# Installation---------------------------------------------------------------
install(TARGETS ${EXE_TARGET_NAME} DESTINATION bin)
install(DIRECTORY src/ DESTINATION src)

message(STATUS "Created target ${EXE_TARGET_NAME} for export ${PROJECT_NAME} v${PROJECT_VERSION}.")

