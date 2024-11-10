# cmake
cmake_minimum_required(VERSION 3.16)

# project
project(genesis_crypt)

# root path
set(ROOT_PATH ..)

# globals
include(${ROOT_PATH}/global.cmake)

# dependencies
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

# includes
include_directories(
  ${ROOT_PATH}/extern/common_thirdparty/openssl_3/x64/include
)

# headers
set(HEADERS
  Common.h
  InnerCrypt.h
  Jwt.h
  Rsa.h
  Tlv.h
  Tools.h
)

# sources
set(SOURCES
  InnerCrypt.cpp
  Jwt.cpp
  Rsa.cpp
  Tlv.cpp
)

# reources
set(RESOURCES
  InnerData.qrc
)

# target
add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES} ${RESOURCES})

# libs
target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Qt${QT_VERSION_MAJOR}::Core
)
