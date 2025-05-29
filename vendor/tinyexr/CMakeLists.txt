# this cmake file is for compile tests.
# Not usable for your cmake app.
cmake_minimum_required(VERSION 3.5)

#---------- REMI ADDITION FOR ALLOWING LINKING -------------
project ("tinyexr")

add_library(tinyexr)
target_sources(tinyexr PRIVATE "src/tinyexr" "src/miniz.c" "include/miniz.h")
target_include_directories(tinyexr PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
#------------------------------------------------------------