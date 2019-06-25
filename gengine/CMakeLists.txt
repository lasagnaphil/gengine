# Threads
find_package(Threads REQUIRED)

# SDL2
find_package(SDL2 REQUIRED)

# GLM
add_subdirectory(deps/glm)

# GLAD
set(GLAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/glad)
add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${GLAD_DIR}/include)

# IMGUI
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui)
add_library(imgui STATIC
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp)
target_include_directories(imgui PUBLIC ${IMGUI_DIR})
target_link_libraries(imgui PUBLIC glad SDL2)

# STB
set(STB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/stb)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${STB_DIR})

# tinyml2
set(TINYXML2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/tinyxml2)
add_library(tinyxml2 STATIC ${TINYXML2_DIR}/tinyxml2.cpp)
target_include_directories(tinyxml2 PUBLIC ${TINYXML2_DIR})

# span-lite
set(SPAN_LITE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/span-lite)
add_library(span-lite INTERFACE)
target_include_directories(span-lite INTERFACE ${SPAN_LITE_DIR})


# Add the source files to the library
file(GLOB RENDER_SOURCES *.cpp)
add_library(gengine STATIC ${RENDER_SOURCES})

# Use this compile definition to use std::shared_ptr instead of generational storage
# target_compile_definitions(gengine PUBLIC USE_SHARED_PTR)

# include/linking
target_include_directories(gengine
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gengine ${CMAKE_DL_LIBS} Threads::Threads SDL2 glad imgui stb glm tinyxml2 span-lite)

# Compile shaders into header files
add_executable(embed_shader embed_shader.cpp)
file(GLOB shaders_src
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.geom")

foreach(shader ${shaders_src})
    get_filename_component(shader_name ${shader} NAME)
    add_custom_target(
            ${shader_name}_build_target
            COMMAND embed_shader ${shader} ${shader}.h
            DEPENDS ${shader}
    )
    add_dependencies(gengine ${shader_name}_build_target)
endforeach()
