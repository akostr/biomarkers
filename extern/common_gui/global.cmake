# cmake
cmake_minimum_required(VERSION 3.16)

# parent project globals
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../global.cmake")
  include(${CMAKE_CURRENT_LIST_DIR}/../../global.cmake)
endif()

# includes
include_directories(
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/../common_core
  ${CMAKE_CURRENT_LIST_DIR}/../common_thirdparty
)
