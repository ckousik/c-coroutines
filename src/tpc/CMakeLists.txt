cmake_minimum_required(VERSION 3.0)

add_library(tpc "")
file(GLOB TPC_INCLUDE_FILES LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/include/tpc *.h)
# target_include_directories(tpc PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(
  tpc
  PUBLIC
  ${PROJECT_SOURCE_DIR}/include/tpc
)

target_sources(tpc
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/executor.c
  ${CMAKE_CURRENT_LIST_DIR}/log.c
  ${CMAKE_CURRENT_LIST_DIR}/queue.c
  ${CMAKE_CURRENT_LIST_DIR}/task.c
  ${CMAKE_CURRENT_LIST_DIR}/waker.c
  # reactor
  ${CMAKE_CURRENT_LIST_DIR}/reactor.c
  ${CMAKE_CURRENT_LIST_DIR}/simple_thread_executor.c
  PUBLIC
  ${TPC_INCLUDE_FILES}
)
