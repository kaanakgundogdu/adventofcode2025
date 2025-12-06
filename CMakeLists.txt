cmake_minimum_required(VERSION 3.15)
project(AdventOfCode2025 CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_subdirectory(day_1)
add_subdirectory(day_2)
add_subdirectory(day_3)
add_subdirectory(day_4)
add_subdirectory(day_5)
add_subdirectory(day_6)