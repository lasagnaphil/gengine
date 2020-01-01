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

add_executable(gengine_demo_spheres_pbr spheres-pbr.cpp)
target_include_directories(gengine_demo_spheres_pbr PUBLIC gengine)
target_link_libraries(gengine_demo_spheres_pbr gengine)
add_dependencies(gengine_demo_spheres_pbr copy_resource_folder)

add_executable(gengine_demo_hw1_human hw1-human.cpp)
target_include_directories(gengine_demo_hw1_human PUBLIC gengine)
target_link_libraries(gengine_demo_hw1_human gengine)
add_dependencies(gengine_demo_hw1_human copy_resource_folder)

add_executable(gengine_demo_hw2_bvh hw2-bvh.cpp)
target_include_directories(gengine_demo_hw2_bvh PUBLIC gengine)
target_link_libraries(gengine_demo_hw2_bvh gengine)
add_dependencies(gengine_demo_hw2_bvh copy_resource_folder)

add_executable(gengine_demo_hw3_ik hw3-ik.cpp)
target_include_directories(gengine_demo_hw3_ik PUBLIC gengine)
target_link_libraries(gengine_demo_hw3_ik gengine)
add_dependencies(gengine_demo_hw3_ik copy_resource_folder)

add_executable(gengine_demo_hw4_fsm hw4-fsm.cpp)
target_include_directories(gengine_demo_hw4_fsm PUBLIC gengine)
target_link_libraries(gengine_demo_hw4_fsm gengine)
add_dependencies(gengine_demo_hw4_fsm copy_resource_folder)

add_executable(gengine_demo_plot plot.cpp)
target_include_directories(gengine_demo_plot PUBLIC gengine)
target_link_libraries(gengine_demo_plot gengine)
add_dependencies(gengine_demo_plot copy_resource_folder)

