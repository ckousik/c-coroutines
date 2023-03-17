cmake_minimum_required(VERSION 3.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(tpc)
add_subdirectory(uring_reactor)
set_target_properties(uring_reactor PROPERTIES LINKER_LANGUAGE C)
