add_library(connection INTERFACE)

target_sources(connection
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/connection.c
)

target_include_directories(connection
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
