include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}")

cmake_minimum_required(VERSION 3.21)
set(NAME BananEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "-gdwarf-4")

message(STATUS "using ${CMAKE_GENERATOR}")
if (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
    if (NOT MINGW_PATH)
        message(FATAL_ERROR "MINGW_PATH not set in .env.cmake")
    endif()
    set(USE_MINGW "True")
    set(CMAKE_C_COMPILER ${MINGW_PATH}/bin/clang.exe)
    set(CMAKE_CXX_COMPILER  ${MINGW_PATH}/bin/clang++.exe)
endif()

project(${NAME} VERSION 0.1)

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
if (DEFINED GLFW_PATH)
    message(STATUS "Using GLFW path specified in .env")
    set(GLFW_INCLUDE_DIRS "${GLFW_PATH}/include")
    if (MSVC)
        set(GLFW_LIB "${GLFW_PATH}/lib-vc2019")
    elseif (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
        message(STATUS "USING MINGW")
        set(GLFW_LIB "${GLFW_PATH}/lib-mingw-w64")
    endif()
else()
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GLFW REQUIRED glfw3)
    message(STATUS "Found GLFW: ${GLFW_LIBRARIES}")
endif()

if (NOT GLFW_FOUND)
    message(FATAL_ERROR "Could not find glfw library!")
else()
    message(STATUS "Using glfw lib at: ${GLFW_LINK_LIBRARIES}")
endif()

# find zlib (needed for openexr)
if (DEFINED ZLIB_PATH)
    set(ZLIB_INCLUDE_DIRS "${ZLIB_PATH}/include")
    set(ZLIB_LIBRARIES "${ZLIB_PATH}/lib")
    set(ZLIB_FOUND "True")
else()
    find_package(ZLIB REQUIRED)
    message(STATUS "Found Zlib: ${ZLIB_LIBRARY}")
endif()

if (NOT ZLIB_FOUND)
    message(FATAL_ERROR "Could not find Zlib library!")
else()
    message(STATUS "Using zlib lib at: ${ZLIB_LIBRARIES}")
endif()

if (DEFINED IMATH_PATH)
    set(IMATH_INCLUDE_DIRS "${IMATH_PATH}/include/Imath")
    set(IMATH_LIBRARIES "${IMATH_PATH}/lib")
    set(IMATH_FOUND "True")
else()
    find_package(Imath REQUIRED)
    message(STATUS "Found Imath: ${IMATH_LIBRARIES}")
endif()

if (NOT IMATH_FOUND)
    message(FATAL_ERROR "Could not find Imath library!")
else()
    message(STATUS "Using Imath lib at: ${IMATH_LIBRARIES}")
endif()

if (DEFINED OPENEXR_PATH)
    set(OPENEXR_INCLUDE_DIRS "${OPENEXR_PATH}/include/OpenEXR")
    set(OPENEXR_LIBRARIES "${OPENEXR_PATH}/lib")
    set(OPENEXR_FOUND "True")
else()
    find_package(OpenEXR REQUIRED)
    message(STATUS "Found OpenEXR: ${OpenEXR_LIBRARIES}")
endif()

if (NOT OPENEXR_FOUND)
    message(FATAL_ERROR "Could not find openexr library!")
else()
    message(STATUS "Using openexr lib at: ${OPENEXR_LIBRARIES}")
endif()

include_directories(/usr/include/stb)

add_library(BananEngine SHARED banan_window.cpp banan_pipeline.cpp banan_device.cpp banan_logger.cpp banan_swap_chain.cpp banan_model.cpp banan_game_object.cpp banan_renderer.cpp banan_camera.cpp banan_buffer.cpp banan_descriptor.cpp banan_image.cpp banan_shadow_mapper.cpp banan_procrastinated_renderer.cpp)
add_executable(BananEngineTest Tests/BananEngineTest.cpp Tests/main.cpp Tests/Systems/SimpleRenderSystem.cpp Tests/Systems/PointLightSystem.cpp Tests/KeyboardMovementController.cpp Tests/Systems/ShadowSystem.cpp Tests/Systems/ProcrastinatedRenderSystem.cpp Tests/Systems/ProcrastinatedOffscreenRenderSystem.cpp Tests/Systems/ComputeSystem.cpp)
target_link_libraries(BananEngineTest PRIVATE BananEngine)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

if (WIN32)
    message(STATUS "CREATING BUILD FOR WINDOWS")

    if (USE_MINGW)
        target_include_directories(${PROJECT_NAME} PUBLIC ${MINGW_PATH}/include)
        target_link_directories(${PROJECT_NAME} PUBLIC ${MINGW_PATH}/lib)
    endif()

    target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR} ${Vulkan_INCLUDE_DIRS} ${ASSIMP_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${GLM_PATH} ${ZLIB_INCLUDE_DIRS} ${OPENEXR_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES} ${GLFW_LIB} ${OPENEXR_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} glfw3 vulkan-1 ${ASSIMP_LIBRARIES} ZLIB::ZLIB)

elseif (UNIX)
    message(STATUS "CREATING BUILD FOR UNIX")
    target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR} ${OPENEXR_INCLUDE_DIRS} ${IMATH_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PUBLIC ${OPENEXR_LIBRARIES} ${IMATH_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} glfw ${Vulkan_LIBRARIES} ${ASSIMP_LIBRARIES} Imath OpenEXR ZLIB::ZLIB)
endif()

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

execute_process(COMMAND glslc ${CMAKE_SOURCE_DIR}/Tests/Shaders/calc_normal_mats.comp -o ${CMAKE_BINARY_DIR}/shaders/calc_normal_mats.comp.spv)

execute_process(COMMAND cp -r ${CMAKE_SOURCE_DIR}/Tests/banan_assets ${CMAKE_BINARY_DIR})

