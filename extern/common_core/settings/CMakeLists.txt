# cmake
cmake_minimum_required(VERSION 3.16)

# project
project(settings)

# root path
set(ROOT_PATH ..)

# globals
include(${ROOT_PATH}/global.cmake)

# dependencies
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core)

# headers
set(HEADERS
  settings.h 
)

# sources
set(SOURCES
  settings.cpp
)

# target
add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

# libs
target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Qt${QT_VERSION_MAJOR}::Core
)
