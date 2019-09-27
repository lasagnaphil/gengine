add_custom_target(copy_resource_folder)
add_custom_command(TARGET copy_resource_folder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources
        ${CMAKE_CURRENT_BINARY_DIR}/resources)

add_executable(gengine_demo_boxes boxes.cpp)
target_include_directories(gengine_demo_boxes PUBLIC gengine)
target_link_libraries(gengine_demo_boxes gengine)
add_dependencies(gengine_demo_boxes copy_resource_folder)

add_executable(gengine_demo_trackball trackball.cpp)
target_include_directories(gengine_demo_trackball PUBLIC gengine)
target_link_libraries(gengine_demo_trackball gengine)
add_dependencies(gengine_demo_trackball copy_resource_folder)

add_executable(gengine_demo_hw1_human hw1-human.cpp)
target_include_directories(gengine_demo_hw1_human PUBLIC gengine)
target_link_libraries(gengine_demo_hw1_human gengine)
add_dependencies(gengine_demo_hw1_human copy_resource_folder)

add_executable(gengine_demo_hw2_bvh hw2-bvh.cpp)
target_include_directories(gengine_demo_hw2_bvh PUBLIC gengine)
target_link_libraries(gengine_demo_hw2_bvh gengine)
add_dependencies(gengine_demo_hw2_bvh copy_resource_folder)
