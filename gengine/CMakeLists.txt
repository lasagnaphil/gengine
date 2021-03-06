# Threads
find_package(Threads REQUIRED)

# SDL2
find_package(SDL2 REQUIRED)

# GLM
if (NOT TARGET glm)
    set(GLM_TEST_ENABLE OFF CACHE BOOL "Turn off GLM tests")
    add_subdirectory(deps/glm)
endif()

if (NOT TARGET glmx)
    add_subdirectory(deps/glmx)
endif()

# Eigen
set(EIGEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/Eigen-3.3.7)
add_library(eigen INTERFACE)
target_include_directories(eigen INTERFACE ${EIGEN_DIR})

# GLAD
set(GLAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/glad)
add_library(glad STATIC ${GLAD_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${GLAD_DIR}/include)
target_link_libraries(glad PUBLIC ${CMAKE_DL_LIBS})

# IMGUI
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui)
add_library(imgui STATIC
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp)
target_include_directories(imgui PUBLIC ${IMGUI_DIR})
target_link_libraries(imgui PUBLIC glad SDL2::Core)

# STB
set(STB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/stb)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${STB_DIR})

# tinyxml2
set(TINYXML2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/tinyxml2)
add_library(tinyxml2 STATIC ${TINYXML2_DIR}/tinyxml2.cpp)
target_include_directories(tinyxml2 PUBLIC ${TINYXML2_DIR})

# tiny_obj_loader
set(TINYOBJLOADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/tiny_obj_loader)
add_library(tinyobjloader STATIC ${TINYOBJLOADER_DIR}/tiny_obj_loader.cc)
target_include_directories(tinyobjloader PUBLIC ${TINYOBJLOADER_DIR})

# span-lite
set(SPAN_LITE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/span-lite)
add_library(span-lite INTERFACE)
target_include_directories(span-lite INTERFACE ${SPAN_LITE_DIR})

# fmt
set(FMT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/fmt)
add_subdirectory(${FMT_DIR} EXCLUDE_FROM_ALL)

# bitsery
set(BITSERY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/bitsery)
add_subdirectory(${BITSERY_DIR} EXCLUDE_FROM_ALL)

# doctest
set(DOCTEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/doctest)
add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE ${DOCTEST_DIR})

option(GENGINE_ENABLE_PHYSX "Enable PhysX" OFF)
option(GENGINE_ENABLE_PHYSX_GPU "Enable PhysX GPU" OFF)

if (GENGINE_ENABLE_PHYSX)
    find_package(PhysX REQUIRED)
endif()

# Add the source files to the library
file(GLOB GENGINE_SOURCES *.cpp)
file(GLOB GENGINE_GLMX_SOURCES glmx/*.cpp)
file(GLOB GENGINE_PHYSX_SOURCES physx/*.cpp)
file(GLOB GENGINE_PLOT_SOURCES plot/*.cpp)
file(GLOB GENGINE_SERIALIZE_SOURCES serialize/*.cpp)
file(GLOB GENGINE_ANIM_SOURCES anim/*.cpp)

if (GENGINE_ENABLE_PHYSX)
    add_library(gengine STATIC ${GENGINE_SOURCES} ${GENGINE_GLMX_SOURCES} ${GENGINE_PLOT_SOURCES} ${GENGINE_SERIALIZE_SOURCES} ${GENGINE_ANIM_SOURCES}
            ${GENGINE_PHYSX_SOURCES})
else()
    add_library(gengine STATIC ${GENGINE_SOURCES} ${GENGINE_GLMX_SOURCES} ${GENGINE_PLOT_SOURCES} ${GENGINE_SERIALIZE_SOURCES} ${GENGINE_ANIM_SOURCES})
endif()

# include/linking
target_include_directories(gengine
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)
target_link_libraries(gengine
        ${CMAKE_DL_LIBS} Threads::Threads SDL2::Main glad imgui stb glm glmx eigen tinyxml2
        tinyobjloader span-lite fmt bitsery doctest)

if (GENGINE_ENABLE_PHYSX)
    target_include_directories(gengine PUBLIC ${PhysX_INCLUDE_DIR} ${PxShared_INCLUDE_DIR})

    if (GENGINE_ENABLE_PHYSX_GPU)
        target_link_directories(gengine PUBLIC /usr/local/cuda/lib64)
        target_link_libraries(gengine debug ${PhysX_LIBRARIES_DEBUG} cuda)
        target_link_libraries(gengine optimized ${PhysX_LIBRARIES_RELEASE} cuda)
    else()
        target_link_libraries(gengine debug ${PhysX_LIBRARIES_DEBUG})
        target_link_libraries(gengine optimized ${PhysX_LIBRARIES_RELEASE})
    endif()
endif()

# Compile shaders into header files
add_executable(embed_shader shaders/embed_shader.cpp)
file(GLOB shaders_src
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.geom")

set(SHADER_HEADER_FILES "")

foreach(shader ${shaders_src})
    get_filename_component(shader_name ${shader} NAME)
    add_custom_command(
            OUTPUT ${shader}.h
            COMMAND embed_shader ${shader} ${shader}.h
            DEPENDS ${shader}
    )
    set(SHADER_HEADER_FILES ${SHADER_HEADER_FILES} ${shader}.h)
endforeach()

add_custom_target(compile_shaders DEPENDS ${SHADER_HEADER_FILES})
add_dependencies(gengine compile_shaders)

add_subdirectory(tests)
