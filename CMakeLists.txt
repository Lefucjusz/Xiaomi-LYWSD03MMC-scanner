cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(Xiaomi-LYWSD03MMC-scanner)

target_sources(app
    PRIVATE 
        src/main.c
)

add_subdirectory(src/scanner)
add_subdirectory(src/xiaomi)
add_subdirectory(src/connection)

target_link_libraries(app
    PRIVATE
        scanner
        xiaomi
        connection
)
