include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
include(CMakePrintHelpers)

message(STATUS "Local .env.cmake: ${LOCAL_ENV}")

cmake_minimum_required(VERSION 3.7)
set(NAME BananEngine)

# This needs to be after the "project" call
#set(CMAKE_CXX_STANDARD 20)
# This will only work if clang is in your path and breaks usage of other compilers
# to force, set these as environment variables
#set(CMAKE_C_COMPILER clang)
#set(CMAKE_CXX_COMPILER clang++)
#set(CMAKE_VERBOSE_MAKEFILE ON)
# This is illegal, per our CMake expert... use "add_compile_options" after the "project" call
#set(CMAKE_CXX_FLAGS "-Wall -Wextra -O3 -gdwarf-4")

message(STATUS "using ${CMAKE_GENERATOR}")
if (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
    # This would be better in a "toolchain" file c.f. cmake documentation
    if (NOT MINGW_PATH)
        message(FATAL_ERROR "MINGW_PATH not set in .env.cmake")
    endif()
    set(USE_MINGW "True")
    set(CMAKE_C_COMPILER ${MINGW_PATH}/bin/clang.exe)
    set(CMAKE_CXX_COMPILER  ${MINGW_PATH}/bin/clang++.exe)
endif()

project(${NAME} VERSION 0.1)
# ... here
set(CMAKE_CXX_STANDARD 20)

# find vulkan
if (DEFINED VULKAN_SDK_PATH)
    set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
    set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib")
    set(Vulkan_FOUND "True")
else()
    find_package(Vulkan REQUIRED)
    message(STATUS "Found Vulkan: ${Vulkan_VERSION}")
endif()

if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
else()
    message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
endif()

# find assimp
if (DEFINED ASSIMP_PATH)
    set(ASSIMP_INCLUDE_DIR "${ASSIMP_PATH}/include")
    set(ASSIMP_LIBRARY_DIR "${ASSIMP_PATH}/lib")
    set(assimp_FOUND "True")
else()
    find_package(assimp REQUIRED)
    message(STATUS "Found ASSIMP: ${ASSIMP_LIBRARIES}")
endif()

if (NOT assimp_FOUND)
    message(FATAL_ERROR "Could not find assimp library!")
else()
    message(STATUS "Using assimp lib at: ${assimp_LIBRARY}")
endif()

# find glfw
if (DEFINED SDL2_PATH)
    message(STATUS "Using SDL2 path specified in .env")
    set(SDL2_INCLUDE_DIRS "${SDL2_PATH}/include")
    if (MSVC)
        set(SDL2_LIBRARIES "${SDL2_PATH}/lib-vc2019")
    elseif (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
        message(STATUS "USING MINGW")
        set(SDL2_LIBRARIES "${SDL2_PATH}/lib-mingw-w64")
    endif()
else()
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
    message(STATUS "Found SDL2: ${SDL2_LIBRARIES}")
endif()

if (NOT SDL2_FOUND)
    message(FATAL_ERROR "Could not find sdl library!")
else()
    message(STATUS "Using sdl lib at: ${SDL2_LIBRARIES}")
endif()

if (DEFINED OPENEXR_PATH)
    set(OPENEXR_INCLUDE_DIRS "${OPENEXR_PATH}/include/OpenEXR")
    set(OPENEXR_LIBRARIES "${OPENEXR_PATH}/lib")
    set(OPENEXR_FOUND "True")
else()
    find_package(OpenEXR REQUIRED)
    # OpenEXR installs Imath when built
    find_package(Imath CONFIG REQUIRED)
    message(STATUS "Found OpenEXR: ${OpenEXR_LIBRARIES}")
endif()

if (NOT OpenEXR_FOUND)
    message(FATAL_ERROR "Could not find openexr library!")
else()
    message(STATUS "Using openexr lib at: ${OPENEXR_LIBRARIES}")
endif()

# This will break Windows builds. use find_package instead.
#include_directories(/usr/include/stb)

# Symbols need to be exported on Windows for a SHARED library to be linkable.
add_library(BananEngine STATIC banan_window.cpp banan_pipeline.cpp banan_device.cpp banan_logger.cpp banan_swap_chain.cpp banan_model.cpp banan_game_object.cpp banan_renderer.cpp banan_camera.cpp banan_buffer.cpp banan_descriptor.cpp banan_image.cpp banan_entity_manager.cpp banan_entity_manager.cpp)
add_executable(BananEngineTest Tests/BananEngineTest.cpp Tests/main.cpp Tests/Systems/SimpleRenderSystem.cpp Tests/Systems/PointLightSystem.cpp Tests/KeyboardMovementController.cpp Tests/Systems/ComputeSystem.cpp Tests/Systems/ProcrastinatedRenderSystem.cpp Tests/Systems/ResolveSystem.cpp)
target_link_libraries(BananEngineTest PRIVATE BananEngine)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
set_property(TARGET BananEngineTest PROPERTY VS_DEBUGGER_ENVIRONMENT "PATH=$ENV{VULKAN_SDK}/bin;$ENV{CMAKE_PREFIX_PATH}/bin;C:/Program Files (x86)/Assimp/bin")

if (WIN32)
    message(STATUS "CREATING BUILD FOR WINDOWS")

    if (USE_MINGW)
        target_include_directories(${PROJECT_NAME} PUBLIC ${MINGW_PATH}/include/)
        target_include_directories(${PROJECT_NAME} PUBLIC ${MINGW_PATH}/include/stb/)
        target_include_directories(${PROJECT_NAME} PUBLIC ${MINGW_PATH}/include/OpenEXR/)
        target_include_directories(${PROJECT_NAME} PUBLIC ${MINGW_PATH}/include/Imath/)
        target_link_directories(${PROJECT_NAME} PUBLIC ${MINGW_PATH}/lib/)
    endif()

    target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR} ${Vulkan_INCLUDE_DIRS} ${ASSIMP_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${GLM_PATH} ${ZLIB_INCLUDE_DIRS} ${OPENEXR_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES} ${GLFW_LIB} ${OPENEXR_LIBRARIES})
    # Use the imported target instead of the explicit _DIRS and don't specify vulkan-1
    # target_link_libraries(${PROJECT_NAME} ${ASSIMP_LIBRARIES} ${SDL2_LIBRARIES} OpenEXR::OpenEXR Vulkan::Vulkan Imath::Imath)
    target_link_libraries(${PROJECT_NAME} assimp::assimp ${SDL2_LIBRARIES} OpenEXR::OpenEXR Vulkan::Vulkan Imath::Imath)

elseif (UNIX)
    message(STATUS "CREATING BUILD FOR UNIX")
    target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR} /usr/include/OpenEXR/ /usr/include/Imath/)
    target_link_directories(${PROJECT_NAME} PUBLIC ${OPENEXR_LIBRARIES} ${IMATH_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} Imath OpenEXR ${Vulkan_LIBRARIES} ${ASSIMP_LIBRARIES} ${SDL2_LIBRARIES})
endif()

# The cross-platform way to make a directory
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory  ${CMAKE_BINARY_DIR}/shaders)
# execute_process(COMMAND mkdir ${CMAKE_BINARY_DIR}/shaders)

# Remove the requirement that glslc is in the path when the SDK is present
if(TARGET Vulkan::glslc)
    set(GLSLC_EXEC Vulkan::glslc)
else()
    set(GLSLC_EXEC glslc)
endif()
    
execute_process(COMMAND ${GLSLC_EXEC} ${CMAKE_SOURCE_DIR}/Tests/Shaders/triangle.frag -o ${CMAKE_BINARY_DIR}/shaders/triangle.frag.spv)
execute_process(COMMAND ${GLSLC_EXEC} ${CMAKE_SOURCE_DIR}/Tests/Shaders/triangle.vert -o ${CMAKE_BINARY_DIR}/shaders/triangle.vert.spv)

execute_process(COMMAND ${GLSLC_EXEC} ${CMAKE_SOURCE_DIR}/Tests/Shaders/point_light.frag -o ${CMAKE_BINARY_DIR}/shaders/point_light.frag.spv)
execute_process(COMMAND ${GLSLC_EXEC} ${CMAKE_SOURCE_DIR}/Tests/Shaders/point_light.vert -o ${CMAKE_BINARY_DIR}/shaders/point_light.vert.spv)

execute_process(COMMAND ${GLSLC_EXEC} ${CMAKE_SOURCE_DIR}/Tests/Shaders/mrt.frag -o ${CMAKE_BINARY_DIR}/shaders/mrt.frag.spv)
execute_process(COMMAND ${GLSLC_EXEC} ${CMAKE_SOURCE_DIR}/Tests/Shaders/mrt.vert -o ${CMAKE_BINARY_DIR}/shaders/mrt.vert.spv)

execute_process(COMMAND ${GLSLC_EXEC} ${CMAKE_SOURCE_DIR}/Tests/Shaders/gbuffer.frag -o ${CMAKE_BINARY_DIR}/shaders/gbuffer.frag.spv)
execute_process(COMMAND ${GLSLC_EXEC} ${CMAKE_SOURCE_DIR}/Tests/Shaders/gbuffer.vert -o ${CMAKE_BINARY_DIR}/shaders/gbuffer.vert.spv)

execute_process(COMMAND ${GLSLC_EXEC} ${CMAKE_SOURCE_DIR}/Tests/Shaders/edge.frag -o ${CMAKE_BINARY_DIR}/shaders/edge.frag.spv)
execute_process(COMMAND ${GLSLC_EXEC} ${CMAKE_SOURCE_DIR}/Tests/Shaders/edge.vert -o ${CMAKE_BINARY_DIR}/shaders/edge.vert.spv)

execute_process(COMMAND ${GLSLC_EXEC} ${CMAKE_SOURCE_DIR}/Tests/Shaders/blend.frag -o ${CMAKE_BINARY_DIR}/shaders/blend.frag.spv)
execute_process(COMMAND ${GLSLC_EXEC} ${CMAKE_SOURCE_DIR}/Tests/Shaders/blend.vert -o ${CMAKE_BINARY_DIR}/shaders/blend.vert.spv)

execute_process(COMMAND ${GLSLC_EXEC} ${CMAKE_SOURCE_DIR}/Tests/Shaders/resolve.frag -o ${CMAKE_BINARY_DIR}/shaders/resolve.frag.spv)
execute_process(COMMAND ${GLSLC_EXEC} ${CMAKE_SOURCE_DIR}/Tests/Shaders/resolve.vert -o ${CMAKE_BINARY_DIR}/shaders/resolve.vert.spv)

execute_process(COMMAND ${GLSLC_EXEC} ${CMAKE_SOURCE_DIR}/Tests/Shaders/calc_normal_mats.comp -o ${CMAKE_BINARY_DIR}/shaders/calc_normal_mats.comp.spv)

# Portable way to copy a directory
execute_process(COMMAND ${CMAKE_COMMAND} -E  copy_directory ${CMAKE_SOURCE_DIR}/Tests/banan_assets ${CMAKE_BINARY_DIR}/banan_assets)

