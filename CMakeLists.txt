include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}")

cmake_minimum_required(VERSION 3.7)
set(NAME BananEngine)
set(CMAKE_CXX_STANDARD 17)

project(${NAME} VERSION 0.1)

if (WIN32)
    if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/vcpkg.exe)
        execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/bootstrap-vcpkg.bat)
    endif()

    set(VCPKG_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/vcpkg.exe)
    set(VCPKG_PLATFORM ":x64-windows")

    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
elseif(UNIX)
    if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/vcpkg)
        execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/bootstrap-vcpkg.sh)
    endif()

    set(VCPKG_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/vcpkg)
    set(VCPKG_PLATFORM ":x64-linux")
endif()

execute_process(COMMAND ${VCPKG_EXECUTABLE} integrate install)
execute_process(COMMAND ${VCPKG_EXECUTABLE} list OUTPUT_VARIABLE VCPKG_INSTALLED_PACKAGES)

if (NOT ${VCPKG_INSTALLED_PACKAGES} MATCHES ".*assimp${VCPKG_PLATFORM}.*")
    execute_process(COMMAND ${VCPKG_EXECUTABLE} install assimp${VCPKG_PLATFORM})
endif()

if (NOT ${VCPKG_INSTALLED_PACKAGES} MATCHES ".*openexr${VCPKG_PLATFORM}.*")
    execute_process(COMMAND ${VCPKG_EXECUTABLE} install openexr${VCPKG_PLATFORM})
endif()

if (NOT ${VCPKG_INSTALLED_PACKAGES} MATCHES ".*sdl2${VCPKG_PLATFORM}.*")
    execute_process(COMMAND ${VCPKG_EXECUTABLE} install sdl2[vulkan]${VCPKG_PLATFORM})
endif()

if (NOT ${VCPKG_INSTALLED_PACKAGES} MATCHES ".*vulkan${VCPKG_PLATFORM}.*")
    execute_process(COMMAND ${VCPKG_EXECUTABLE} install vulkan${VCPKG_PLATFORM})
endif()

if (NOT ${VCPKG_INSTALLED_PACKAGES} MATCHES ".*stb${VCPKG_PLATFORM}.*")
    execute_process(COMMAND ${VCPKG_EXECUTABLE} install stb${VCPKG_PLATFORM})
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

find_package(assimp CONFIG REQUIRED)
find_package(OpenEXR CONFIG REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Stb REQUIRED)
find_package(Vulkan REQUIRED)

if (NOT TARGET Vulkan::glslangValidator)
    message(FATAL_ERROR "GLSL Compiler not found")
endif()

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/Tests/Shaders/*.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tests/Shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/Tests/Shaders/*.comp"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
        COMMAND Vulkan::glslangValidator -V ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)

set(BananLib banan_window.cpp banan_pipeline.cpp banan_device.cpp banan_logger.cpp banan_swap_chain.cpp banan_model.cpp banan_game_object.cpp banan_renderer.cpp banan_camera.cpp banan_buffer.cpp banan_descriptor.cpp banan_image.cpp)
set(BananTest Tests/BananEngineTest.cpp Tests/main.cpp Tests/Systems/PointLightSystem.cpp Tests/KeyboardMovementController.cpp Tests/Systems/ComputeSystem.cpp Tests/Systems/ProcrastinatedRenderSystem.cpp Tests/Systems/ResolveSystem.cpp Tests/Systems/PointShadowSystem.cpp Tests/Systems/PointShadowSystem.h)

add_library(BananEngine SHARED ${BananLib})
add_executable(BananEngineTest ${BananTest})
target_link_libraries(BananEngineTest PRIVATE BananEngine)
target_include_directories(BananEngineTest PRIVATE ${CMAKE_SOURCE_DIR})

set(CMAKE_C_FLAGS $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Werror -Wpedantic -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-unused-parameter -Wno-unused-function -Wno-switch-enum -Wno-unused-macros>)
set(CMAKE_C_FLAGS_DEBUG $<$<CXX_COMPILER_ID:MSVC>:/DEBUG> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-g>)
set(CMAKE_C_FLAGS_RELEASE $<$<CXX_COMPILER_ID:MSVC>:/O2> $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-O3>)

add_dependencies(BananEngineTest Shaders)

add_custom_command(TARGET BananEngineTest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:BananEngineTest>/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${PROJECT_BINARY_DIR}/shaders"
    "$<TARGET_FILE_DIR:BananEngineTest>/shaders"
)

target_link_libraries(BananEngine assimp::assimp OpenEXR::OpenEXR Vulkan::Vulkan $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main> $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>)
include_directories(${ASSIMP_INCLUDE_DIRS} ${OpenEXR_INCLUDE_DIRS} ${Stb_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS})

