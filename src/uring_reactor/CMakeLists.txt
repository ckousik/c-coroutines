cmake_minimum_required(VERSION 3.0)

add_library(uring_reactor "")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB URING_REACTOR_INCLUDE_FILES LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/include/uring_reactor *.h)
# --------- liburing dependency -----------------

target_include_directories(
  uring_reactor
  PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/../third_party/liburing/src/include/
)

target_link_libraries(
  uring_reactor
  ${CMAKE_CURRENT_LIST_DIR}/../third_party/liburing/src/liburing.a
)

# -------------------------------------------------

# ----------hashmap dependency --------------------

target_sources(
  uring_reactor
  PRIVATE
  "../third_party/c-hashmap/map.c"
  "../third_party/c-hashmap/map.h"
)

add_compile_definitions(__HASHMAP_REMOVABLE)

target_include_directories(
  uring_reactor
  PRIVATE
  "../third_party/c-hashmap/"
)

# -------------------------------------------------

target_include_directories(
  uring_reactor
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/internal>
)

target_include_directories(
  uring_reactor
  PUBLIC
  ${PROJECT_SOURCE_DIR}/include
)

target_sources(
  uring_reactor
  PUBLIC
  ${URING_REACTOR_INCLUDE_FILES}
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/internal/uring_reactor.h
  ${CMAKE_CURRENT_LIST_DIR}/uring_reactor.c
  ${CMAKE_CURRENT_LIST_DIR}/task_timer.c
)

target_link_libraries(
  uring_reactor
  tpc
)
