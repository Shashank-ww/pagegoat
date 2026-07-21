cmake_minimum_required(VERSION 3.20)

project(PageGoat
    VERSION 0.1.0
    LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(pagegoat
    apps/cli/main.c

    engine/coregoat/context.c
    engine/coregoat/status.c
)

target_include_directories(pagegoat
    PRIVATE
        include
)