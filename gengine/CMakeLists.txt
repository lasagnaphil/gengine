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

# span-lite
set(SPAN_LITE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/span-lite)
add_library(span-lite INTERFACE)
target_include_directories(span-lite INTERFACE ${SPAN_LITE_DIR})

file(GLOB RENDER_SOURCES *.cpp)

add_library(gengine STATIC ${RENDER_SOURCES})

target_include_directories(gengine
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(gengine ${CMAKE_DL_LIBS} Threads::Threads SDL2 glad imgui stb glm span-lite)

add_custom_command(
        TARGET gengine POST_BUILD
        DEPENDS ${CMAKE_SOURCE_DIR}/shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:gengine>/shaders
        COMMENT "Copying shaders" VERBATIM
)
add_custom_command(
        TARGET gengine POST_BUILD
        DEPENDS ${CMAKE_SOURCE_DIR}/shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources $<TARGET_FILE_DIR:gengine>/resources
        COMMENT "Copying resources" VERBATIM
)