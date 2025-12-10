cmake_minimum_required(VERSION 3.15)
project(AdventOfCode2025 CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(common)

foreach(day_num RANGE 1 25)
    set(day_dir "day_${day_num}")
    
    # Only add the directory if the folder actually exists
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${day_dir}")
        message(STATUS "Adding ${day_dir}")
        add_subdirectory(${day_dir})
    endif()

    
endforeach()