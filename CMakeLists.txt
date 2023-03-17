cmake_minimum_required(VERSION 3.0)

project(tpc)

# C compiler options
set(CMAKE_C_FLAGS "-Wall -std=c99")

add_subdirectory(src)
add_executable(main main.c)
target_link_libraries(main tpc)
target_link_libraries(main uring_reactor)
target_include_directories(main PUBLIC ${PROJECT_SOURCE_DIR}/include)
