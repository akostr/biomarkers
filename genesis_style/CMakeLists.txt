# cmake
cmake_minimum_required(VERSION 3.16)

# project
project(genesis_style)

# root path
set(ROOT_PATH ..)

# globals
include(${ROOT_PATH}/global.cmake)

# dependencies
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)

# headers
set(HEADERS
  app_proxy_style.h
  constants.h
  graphics_effect_focused_input_glow.h
  style.h
  utils.h 
)

# sources
set(SOURCES
  app_proxy_style.cpp
  constants.cpp
  graphics_effect_focused_input_glow.cpp
  style.cpp
  utils.cpp 
)

# reources
set(RESOURCES
  genesis_style.qrc
)

# target
add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES} ${RESOURCES})

# libs
target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Widgets
)
