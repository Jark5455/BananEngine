cmake_minimum_required(VERSION 3.21)
project(BananEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS -gdwarf-4)

include_directories(/usr/include/stb)

add_library(BananEngine SHARED banan_window.cpp banan_pipeline.cpp banan_device.cpp banan_logger.cpp banan_swap_chain.cpp banan_model.cpp banan_game_object.cpp banan_renderer.cpp banan_camera.cpp banan_buffer.cpp banan_descriptor.cpp banan_image.cpp banan_shadow_mapper.cpp banan_procrastinated_renderer.cpp)
add_executable(BananEngineTest Tests/BananEngineTest.cpp Tests/main.cpp Tests/Systems/SimpleRenderSystem.cpp Tests/Systems/PointLightSystem.cpp Tests/KeyboardMovementController.cpp Tests/Systems/ShadowSystem.cpp Tests/Systems/ProcrastinatedRenderSystem.cpp Tests/Systems/ProcrastinatedRenderSystem.h Tests/Systems/ProcrastinatedOffscreenRenderSystem.cpp Tests/Systems/ProcrastinatedOffscreenRenderSystem.h)

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})

find_package(PkgConfig REQUIRED)

find_package(assimp REQUIRED)
target_link_libraries(${PROJECT_NAME} ${ASSIMP_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${ASSIMP_INCLUDE_DIRS})

pkg_search_module(GLFW REQUIRED glfw3)
target_link_libraries(${PROJECT_NAME} ${GLFW_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${GLFW_INCLUDE_DIRS})

target_link_libraries(BananEngineTest PRIVATE BananEngine)

execute_process(COMMAND mkdir ${CMAKE_BINARY_DIR}/shaders)
execute_process(COMMAND glslc ${CMAKE_SOURCE_DIR}/Tests/Shaders/triangle.frag -o ${CMAKE_BINARY_DIR}/shaders/triangle.frag.spv)
execute_process(COMMAND glslc ${CMAKE_SOURCE_DIR}/Tests/Shaders/triangle.vert -o ${CMAKE_BINARY_DIR}/shaders/triangle.vert.spv)

execute_process(COMMAND glslc ${CMAKE_SOURCE_DIR}/Tests/Shaders/point_light.frag -o ${CMAKE_BINARY_DIR}/shaders/point_light.frag.spv)
execute_process(COMMAND glslc ${CMAKE_SOURCE_DIR}/Tests/Shaders/point_light.vert -o ${CMAKE_BINARY_DIR}/shaders/point_light.vert.spv)

execute_process(COMMAND glslc ${CMAKE_SOURCE_DIR}/Tests/Shaders/shadow.frag -o ${CMAKE_BINARY_DIR}/shaders/shadow.frag.spv)
execute_process(COMMAND glslc ${CMAKE_SOURCE_DIR}/Tests/Shaders/shadow.vert -o ${CMAKE_BINARY_DIR}/shaders/shadow.vert.spv)

execute_process(COMMAND glslc ${CMAKE_SOURCE_DIR}/Tests/Shaders/mrt.frag -o ${CMAKE_BINARY_DIR}/shaders/mrt.frag.spv)
execute_process(COMMAND glslc ${CMAKE_SOURCE_DIR}/Tests/Shaders/mrt.vert -o ${CMAKE_BINARY_DIR}/shaders/mrt.vert.spv)

execute_process(COMMAND glslc ${CMAKE_SOURCE_DIR}/Tests/Shaders/procrastinated.frag -o ${CMAKE_BINARY_DIR}/shaders/procrastinated.frag.spv)
execute_process(COMMAND glslc ${CMAKE_SOURCE_DIR}/Tests/Shaders/procrastinated.vert -o ${CMAKE_BINARY_DIR}/shaders/procrastinated.vert.spv)

execute_process(COMMAND cp -r ${CMAKE_SOURCE_DIR}/Tests/banan_assets ${CMAKE_BINARY_DIR})

