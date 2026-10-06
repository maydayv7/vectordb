file(GLOB_RECURSE VECTORDB_FORMAT_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/include/*.hpp"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/examples/*.cpp"
    "${PROJECT_SOURCE_DIR}/tests/*.cpp"
)

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format-21 clang-format)
if(CLANG_FORMAT_EXECUTABLE)
    add_custom_target(format
        COMMAND "${CLANG_FORMAT_EXECUTABLE}" -i ${VECTORDB_FORMAT_FILES}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        VERBATIM
    )
    add_custom_target(format-check
        COMMAND "${CLANG_FORMAT_EXECUTABLE}" --dry-run --Werror ${VECTORDB_FORMAT_FILES}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        VERBATIM
    )
endif()

find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy-21 clang-tidy)
if(CLANG_TIDY_EXECUTABLE)
    set(VECTORDB_ANALYSIS_FILES ${VECTORDB_FORMAT_FILES})
    list(FILTER VECTORDB_ANALYSIS_FILES INCLUDE REGEX "\\.cpp$")
    if(NOT BUILD_TESTING)
        list(FILTER VECTORDB_ANALYSIS_FILES EXCLUDE REGEX "/tests/")
    endif()

    add_custom_target(tidy
        COMMAND "${CLANG_TIDY_EXECUTABLE}" -p "${PROJECT_BINARY_DIR}" ${VECTORDB_ANALYSIS_FILES}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        VERBATIM
    )
endif()
