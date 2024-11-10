# cmake
cmake_minimum_required(VERSION 3.16)

# project
project(json_utils)

# root path
set(ROOT_PATH ../..)

# globals
include(${ROOT_PATH}/global.cmake)

# dependencies
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui)

# includes 
include_directories(
  ${Qt6Gui_PRIVATE_INCLUDE_DIRS}
  ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
)

# includes
include_directories(
  ${ROOT_PATH}/../common_thirdparty/rapidjson/include
)

# defines
add_compile_definitions(
  RAPIDJSON_HAS_STDSTRING
)

# headers
set(HEADERS
  rapid.h
  jsoncpp.h
  base.h
  files.h
)

# sources
set(SOURCES
  rapid.cpp
  jsoncpp.cpp
  base.cpp
  files.cpp
)

# target
add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

# libs
target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
)
