# C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Cmake target paths
if (UNIX OR APPLE)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY                  ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG            ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/bin/debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE          ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/bin/release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO   ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/bin/release_with_debug_info)
else ()
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY                  ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG            ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/lib/debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE          ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/lib/release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO   ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/lib/release_with_debug_info)
endif (UNIX OR APPLE)

set(CMAKE_PDB_OUTPUT_DIRECTORY_DEBUG                ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/bin/debug)
set(CMAKE_PDB_OUTPUT_DIRECTORY_RELEASE              ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/bin/release)
set(CMAKE_PDB_OUTPUT_DIRECTORY_RELWITHDEBINFO       ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/bin/release_with_debug_info)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY                  ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG            ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/lib/debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE          ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/lib/release)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO   ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/lib/release_with_debug_info)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY                  ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG            ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE          ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/bin/release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO   ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/bin/release_with_debug_info)

if (UNIX OR APPLE)
set(PLATFORM_LIBRARY_PREFIX "lib")
set(PLATFORM_LIBRARY_SUFFIX ".a")
else ()
set(PLATFORM_BINARY_SUFFIX  ".exe")
set(PLATFORM_LIBRARY_PREFIX "")
set(PLATFORM_LIBRARY_SUFFIX ".lib")
endif (UNIX OR APPLE)

if (APPLE)
set(PLATFORM_BUNDLE MACOSX_BUNDLE)
else ()
endif (APPLE)

# Debug info
if (APPLE)
set (CMAKE_CXX_FLAGS_DEBUG            "${CMAKE_CXX_FLAGS_DEBUG}           -g")
set (CMAKE_CXX_FLAGS_RELEASE          "${CMAKE_CXX_FLAGS_RELEASE}         -g")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO   "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}  -g")
endif ()

if (WIN32)
set (CMAKE_CXX_FLAGS_DEBUG            "${CMAKE_CXX_FLAGS_DEBUG}           /Zi")
set (CMAKE_CXX_FLAGS_RELEASE          "${CMAKE_CXX_FLAGS_RELEASE}         /Zi")
set (CMAKE_CXX_FLAGS_RELWITHDEBINFO   "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}  /Zi")

set (CMAKE_EXE_LINKER_FLAGS           "${CMAKE_EXE_LINKER_FLAGS}          /DEBUG")
set (CMAKE_SHARED_LINKER_FLAGS        "${CMAKE_SHARED_LINKER_FLAGS}       /DEBUG")
endif ()

if (APPLE)
set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")
endif (APPLE)

# Qt modules
set(QT6_DIR_CMAKE_PREFIX_PATH $ENV{QT6_DIR_CMAKE_PREFIX_PATH})
if ("${QT6_DIR_CMAKE_PREFIX_PATH}" STREQUAL "")
if (PLATFORM_WASM)
set(QT6_DIR_CMAKE_PREFIX_PATH $ENV{QT6_DIR_WASM})
else ()
set(QT6_DIR_CMAKE_PREFIX_PATH $ENV{QT6_DIR})
endif ()
endif ()
set(CMAKE_PREFIX_PATH ${QT6_DIR_CMAKE_PREFIX_PATH})

message(STATUS "===============")
message(STATUS "Qt envirnoment:")
message(STATUS "===============")
message(STATUS "$QT6_DIR_CMAKE_PREFIX_PATH            : $ENV{QT6_DIR_CMAKE_PREFIX_PATH}")
message(STATUS "$QT6_DIR                              : $ENV{QT6_DIR}")
message(STATUS "$QT6_DIR_WASM                         : $ENV{QT6_DIR_WASM}")
message(STATUS "QT6_DIR_CMAKE_PREFIX_PATH resolved to : ${QT6_DIR_CMAKE_PREFIX_PATH}")

# rpath
if (APPLE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH            "@executable_path/../Frameworks/")
endif ()

# Qt features
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt dir
get_target_property(QMAKE_EXECUTABLE Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
get_filename_component(QT_BIN_DIR "${QMAKE_EXECUTABLE}" DIRECTORY)

# deployqt
if (APPLE)
  find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${QT_BIN_DIR}")
  set(PLATFORM_DEPLOYQT_EXECUTABLE ${MACDEPLOYQT_EXECUTABLE})
else ()
  find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
  set(PLATFORM_DEPLOYQT_EXECUTABLE ${WINDEPLOYQT_EXECUTABLE})
endif ()

# definitions / common
add_compile_definitions(
  WIN32_LEAN_AND_MEAN
  RAPIDJSON_HAS_STDSTRING
  SETTINGS_WRITABLE_LOCATION_KEY="Genesis"
)

# definitions / obsolete
set(DISABLE_UBJSON                      1)
set(DISABLE_UBJSON_FORMAT               1)
set(DISABLE_BOOST                       1)
set(DISABLE_CUSTOM_WINDOWS              1)
set(DISABLE_STANDARD_MODELS_SERIALIZING 1)
set(DISABLE_STANDARD_VIEWS              1)
set(DISABLE_CONCURRENT                  1)
set(DISABLE_XLNT                        1)
set(DISABLE_CYBER_FRAC_STYLE            1)

#add_compile_definitions(
#  DISABLE_UBJSON
#  DISABLE_UBJSON_FORMAT
#  DISABLE_BOOST
#  DISABLE_CUSTOM_WINDOWS
#  DISABLE_STANDARD_MODELS_SERIALIZING
#  DISABLE_STANDARD_VIEWS
#  DISABLE_CONCURRENT
#  DISABLE_XLNT
#  DISABLE_CYBER_FRAC_STYLE
#)

# definitions / genesis specific
set(
  COMPILE_DEFS ${COMPILE_DEFS}
  KEY_CLOACK
#  JWT_ENABLED
  DISABLE_UBJSON
  DISABLE_UBJSON_FORMAT
  DISABLE_BOOST
  DISABLE_CUSTOM_WINDOWS
  DISABLE_STANDARD_MODELS_SERIALIZING
  DISABLE_STANDARD_VIEWS
  DISABLE_CONCURRENT
  DISABLE_XLNT
  DISABLE_CYBER_FRAC_STYLE
)
#WARNING If both of JWT_ENABLED and KEY_CLOACK enabled, JWT_ENABLED will be disabled
if((KEY_CLOACK IN_LIST COMPILE_DEFS) AND (JWT_ENABLED IN_LIST COMPILE_DEFS))
  list(REMOVE_ITEM COMPILE_DEFS JWT_ENABLED)
endif()

add_compile_definitions(
  ${COMPILE_DEFS}
)

# platform specific
if (PLATFORM_WASM OR PUBLIC)
  set(DISABLE_SETTINGS_WRITABLE_LOCATION  ON)
  add_compile_definitions(DISABLE_SETTINGS_WRITABLE_LOCATION)
endif ()



# includes
include_directories(
  ${PROJECT_SOURCE_DIR}/${ROOT_PATH}
  ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/extern
  ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/extern/common_core
  ${PROJECT_SOURCE_DIR}/${ROOT_PATH}/extern/common_gui/
  $ENV{EMSDK}/upstream/emscripten/cache/sysroot/include
)
