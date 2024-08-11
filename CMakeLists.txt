cmake_minimum_required(VERSION 3.15)
project(BananEngine VERSION 0.1)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /W4 /WX")
    set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi /Ob0 /Od /RTC1")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra -Werror")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -gdwarf-3 -fsanitize=address")
endif()

include_directories(${PROJECT_SOURCE_DIR}/core)
add_subdirectory("core")

include_directories(${PROJECT_SOURCE_DIR}/stlext)
add_subdirectory("stlext")

include_directories(${PROJECT_SOURCE_DIR}/test)
add_subdirectory("test")

