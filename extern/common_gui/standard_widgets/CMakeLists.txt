# cmake
cmake_minimum_required(VERSION 3.16)

# project
project(standard_widgets)

# root path
set(ROOT_PATH ..)

# globals
include(${ROOT_PATH}/global.cmake)

# globals / conditional
if (NOT DEFINED DISABLE_SETTINGS)
  if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/${ROOT_PATH}/../common_core/settings/CMakeLists.txt.include")
    include(${CMAKE_CURRENT_LIST_DIR}/${ROOT_PATH}/../common_core//settings/CMakeLists.txt.include)
  endif ()
endif ()

if (NOT DEFINED DISABLE_CUSTOM_WINDOWS)
  if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/${ROOT_PATH}/custom_windows/CMakeLists.txt.include")
    include(${CMAKE_CURRENT_LIST_DIR}/${ROOT_PATH}/custom_windows/CMakeLists.txt.include)
  endif ()    
endif ()

if (NOT DEFINED DISABLE_CYBER_FRAC_STYLE)
  include_directories(
    ${ROOT_PATH}/standard_style
  )
endif ()

# dependencies
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets)

# headers
set(HEADERS
  collapsed_widgets_view.h
  code_editor.h
  color_select_button.h
  elided_label.h
  icon_cache.h
  persistent_menu.h
  plain_scroll_area.h
  progress_dialog.h
  range_slider.h
  range_widget.h
  switch_widget.h
  tree_view_combobox.h
  tumbler.h
  wait_indicator.h
  warnings_frame.h
)

# sources
set(SOURCES
  collapsed_widgets_view.cpp
  code_editor.cpp
  color_select_button.cpp
  elided_label.cpp
  icon_cache.cpp
  persistent_menu.cpp
  plain_scroll_area.cpp
  progress_dialog.cpp
  range_slider.cpp
  range_widget.cpp
  switch_widget.cpp
  tree_view_combobox.cpp
  tumbler.cpp
  wait_indicator.cpp
  warnings_frame.cpp
)

# target
add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

# libs
target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Widgets
)
