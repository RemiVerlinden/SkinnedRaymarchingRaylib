cmake_minimum_required(VERSION 3.8)
project("raygizmo")

# This section configures MSVC (Visual Studio) to enable "Edit and Continue" debugging

if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Determine the project's root directory and check if it's the main project

set(RGIZMO_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(RGIZMO_IS_MAIN ON)
else()
    set(RGIZMO_IS_MAIN OFF)
endif()

# Project configuration options

option(RGIZMO_BUILD_SHARED "Build raygizmo as a shared library" OFF)
option(RGIZMO_BUILD_EXAMPLES "Build the examples" ${RGIZMO_IS_MAIN})

# Include standard cmake scripts

include(CheckLibraryExists)

# Submodule check for raylib

set(RAYLIB_PATH "${RGIZMO_ROOT_PATH}/external/raylib")

if(EXISTS "${RAYLIB_PATH}/CMakeLists.txt")
  if(RGIZMO_BUILD_SHARED)
   set(BUILD_SHARED_LIBS ON)
  endif() 
    message(STATUS "Submodule raylib found and valid.")
    add_subdirectory("${RAYLIB_PATH}")
else()
    find_package(raylib QUIET)
    if(TARGET raylib)
        message(STATUS "raylib found via find_package.")
    else()
        message(FATAL_ERROR "The 'raylib' submodule is missing or incomplete, and 'find_package(raylib)' failed. Please make sure the submodule is initialized and updated (git submodule update --init --recursive).")
    endif()
endif()

# Create the raygizmo library by adding all the source files

set(RGIZMO_LIB_TYPE STATIC)

if(RGIZMO_BUILD_SHARED)
    set(RGIZMO_LIB_TYPE SHARED)
    add_definitions(-DRGIZMO_BUILD_SHARED)
endif()

add_library(${PROJECT_NAME} ${RGIZMO_LIB_TYPE}
    "${RGIZMO_ROOT_PATH}/src/raygizmo.c"
)

# Link project dependencies

target_link_libraries(${PROJECT_NAME} PUBLIC raylib)

check_library_exists(m cos "" HAVE_LIB_M)

if(HAVE_LIB_M)
    target_link_libraries(${PROJECT_NAME} PUBLIC m)
endif()

# Specify the include directories needed for the project

target_include_directories(${PROJECT_NAME} PUBLIC
    "${RGIZMO_ROOT_PATH}/include"
    "${RGIZMO_ROOT_PATH}/external/raylib/src"
)

# Ensure the C standard is set for MSVC if using CMake version greater than 3.12

if(CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET raygizmo PROPERTY C_STANDARD 99)
endif()

# Example configuration

if(RGIZMO_BUILD_EXAMPLES)
    include("${RGIZMO_ROOT_PATH}/examples/CMakeLists.txt")
endif()
