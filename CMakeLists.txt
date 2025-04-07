cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(Xiaomi-LYWSD03MMC-scanner)

target_sources(app
    PRIVATE 
        src/main.c
)

add_subdirectory(src/scanner)

target_link_libraries(app
    PRIVATE
        scanner
)
