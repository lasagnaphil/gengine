cmake_minimum_required(VERSION 3.10)
project(gengine)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)

add_subdirectory(gengine)
add_subdirectory(demo)
add_subdirectory(viewer)