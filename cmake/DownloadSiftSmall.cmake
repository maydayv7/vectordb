cmake_minimum_required(VERSION 3.20)

get_filename_component(DATA_DIR "${CMAKE_CURRENT_LIST_DIR}/../data" ABSOLUTE)
file(MAKE_DIRECTORY "${DATA_DIR}")

file(DOWNLOAD
    "ftp://ftp.irisa.fr/local/texmex/corpus/siftsmall.tar.gz"
    "${DATA_DIR}/siftsmall.tar.gz"
    EXPECTED_HASH SHA256=b8f1e59b20319ac44279d5251706909dd3a5b8ca5ce2a11ddb1e73902252770e
    INACTIVITY_TIMEOUT 30
    TIMEOUT 600
    SHOW_PROGRESS
)

file(ARCHIVE_EXTRACT
    INPUT "${DATA_DIR}/siftsmall.tar.gz"
    DESTINATION "${DATA_DIR}"
    PATTERNS
        "siftsmall/siftsmall_base.fvecs"
        "siftsmall/siftsmall_query.fvecs"
        "siftsmall/siftsmall_groundtruth.ivecs"
)

message(STATUS "SIFT-small is ready in ${DATA_DIR}/siftsmall")
