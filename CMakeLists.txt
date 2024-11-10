# cmake
cmake_minimum_required(VERSION 3.16)

# project
project(GenesisProject)

set(DISABLE_SETTINGS_WRITABLE_LOCATION  ON)
add_compile_definitions(DISABLE_SETTINGS_WRITABLE_LOCATION)



# genesis libs / style
add_subdirectory(genesis_style)


# extern / core
add_subdirectory(extern/common_core/settings)
add_subdirectory(extern/common_core/standard_models)


if (NOT DEFINED DISABLE_SETTINGS_WRITABLE_LOCATION)
  # extern / thirdparty
  add_subdirectory(extern/common_thirdparty/miniz)
  # extern / core / conditional
  add_subdirectory(extern/common_core/utils/json_utils)
  # genesis libs / crypt
  add_subdirectory(genesis_crypt)
endif ()

# extern / gui
add_subdirectory(extern/common_gui/standard_widgets)

# RenderDoc API
set(RENDERDOC_TEST  0)
if (RENDERDOC_TEST)
    add_compile_definitions(RENDERDOC_API_DEBUG)
    include_directories("C:/Program Files/RenderDoc")
    add_subdirectory(RenderDoc)
endif ()


# client
add_subdirectory(genesis/client)

# dependencies
if (NOT DEFINED DISABLE_SETTINGS_WRITABLE_LOCATION)
  add_dependencies(
    # target
    Genesis

    # genesis
    genesis_crypt
    genesis_style

    # thirdparty
    miniz

    # core
    settings
    standard_models
    json_utils

    # gui
    standard_widgets
  )
else()
  add_dependencies(
    # target
    Genesis

    # genesis
#    genesis_crypt
    genesis_style

    # thirdparty
  #  miniz

    # core
    settings
    standard_models
  #  json_utils

    # gui
    standard_widgets
  )
endif()

if(WIN32)
  if (NOT CONSOLE_ENABLED)
    set_property(TARGET Genesis PROPERTY WIN32_EXECUTABLE true)
  endif()
endif()

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()
# print_all_variables()
