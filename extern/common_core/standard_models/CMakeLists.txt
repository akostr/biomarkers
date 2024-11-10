# cmake
cmake_minimum_required(VERSION 3.16)

# project
project(standard_models)

# root path
set(ROOT_PATH ..)

# globals
include(${ROOT_PATH}/global.cmake)

# globals / conditional
if (NOT DEFINED DISABLE_SETTINGS)
  if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/${ROOT_PATH}/settings/CMakeLists.txt.include")
    include(${CMAKE_CURRENT_LIST_DIR}/${ROOT_PATH}/settings/CMakeLists.txt.include)
  endif ()
endif ()

if (NOT DEFINED DISABLE_UBJSON)
  if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/${ROOT_PATH}/utils/json_utils/CMakeLists.txt.include")
    include(${CMAKE_CURRENT_LIST_DIR}/${ROOT_PATH}/utils/json_utils/CMakeLists.txt.include)
  endif ()  
endif ()

# dependencies
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

# includes 
include_directories(
  ${Qt6Core_PRIVATE_INCLUDE_DIRS}
  ${Qt5Core_PRIVATE_INCLUDE_DIRS}
)

# headers
set(HEADERS
  abstract_uom_model.h
  standard_item_model.h
  standard_item_model_base.h
  standard_item_model_base_p.h
  standard_uom_model.h
  uom.h
  uom_settings.h
)

# sources
set(SOURCES
  abstract_uom_model.cpp 
  standard_item_model.cpp
  standard_item_model_base.cpp
  standard_uom_model.cpp
  uom.cpp
  uom_settings.cpp
)

# target
add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

# libs
target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::CorePrivate
)
