file(GLOB GENGINE_VIEWER_SOURCES *.cpp)
add_executable(gengine_viewer ${GENGINE_VIEWER_SOURCES})

target_include_directories(gengine_viewer PUBLIC gengine)

target_link_libraries(gengine_viewer gengine)
