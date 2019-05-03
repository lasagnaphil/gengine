file(GLOB GENGINE_DEMO_SOURCES *.cpp)
add_executable(gengine_demo ${GENGINE_DEMO_SOURCES})

target_include_directories(gengine_demo PUBLIC gengine)

target_link_libraries(gengine_demo gengine)
