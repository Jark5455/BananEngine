cmake_minimum_required(VERSION 3.15)
project(BananEngine VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

include_directories(${PROJECT_SOURCE_DIR}/core)
add_subdirectory("core")

include_directories(${PROJECT_SOURCE_DIR}/test)
add_subdirectory("test")

