cmake_minimum_required(VERSION 3.14)
project(gengine)
set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

add_subdirectory(gengine)

file(GLOB GENGINE_DEMO_SOURCES *.cpp)
add_executable(gengine_demo ${GENGINE_DEMO_SOURCES})

target_include_directories(gengine_demo PUBLIC gengine)

target_link_libraries(gengine_demo ${CMAKE_DL_LIBS} Threads::Threads)
target_link_libraries(gengine_demo gengine)
