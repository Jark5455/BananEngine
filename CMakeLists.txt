cmake_minimum_required(VERSION 3.21)
project(BananEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_library(BananEngine SHARED banan_window.cpp banan_pipeline.cpp banan_device.cpp banan_logger.cpp banan_swap_chain.cpp banan_model.cpp)
add_executable(BananEngineTest Tests/BananEngineTest.cpp Tests/main.cpp)

find_package(Boost REQUIRED COMPONENTS log)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIR})

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})

find_package(PkgConfig REQUIRED)

pkg_search_module(GLFW REQUIRED glfw3)
target_link_libraries(${PROJECT_NAME} ${GLFW_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${GLFW_INCLUDE_DIRS})

target_link_libraries(BananEngineTest PRIVATE BananEngine)

execute_process(COMMAND mkdir ${CMAKE_BINARY_DIR}/shaders)
execute_process(COMMAND glslc ${CMAKE_SOURCE_DIR}/Tests/Shaders/triangle.frag -o ${CMAKE_BINARY_DIR}/shaders/triangle.frag.spv)
execute_process(COMMAND glslc ${CMAKE_SOURCE_DIR}/Tests/Shaders/triangle.vert -o ${CMAKE_BINARY_DIR}/shaders/triangle.vert.spv)

