add_library(scanner INTERFACE)

target_sources(scanner
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/scanner.c
)

target_include_directories(scanner
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
