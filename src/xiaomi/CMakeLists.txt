add_library(xiaomi INTERFACE)

target_sources(xiaomi
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/xiaomi.c
)

target_include_directories(xiaomi
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
