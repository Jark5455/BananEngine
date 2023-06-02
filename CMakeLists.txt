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

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(VCPKG_PLATFORM ":x64-windows")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(VCPKG_PLATFORM ":x86-windows")
    endif()

    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
elseif(UNIX)
    if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/vcpkg)
        execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/bootstrap-vcpkg.sh)
    endif()

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(VCPKG_PLATFORM ":x64-linux")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(VCPKG_PLATFORM ":x86-linux")
    endif()

    set(VCPKG_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/vcpkg)
endif()

execute_process(COMMAND ${VCPKG_EXECUTABLE} list OUTPUT_VARIABLE VCPKG_INSTALLED_PACKAGES)

if (NOT ${VCPKG_INSTALLED_PACKAGES} MATCHES ".*assimp${VCPKG_PLATFORM}.*")
    execute_process(COMMAND ${VCPKG_EXECUTABLE} install assimp${VCPKG_PLATFORM})
endif()

if (NOT ${VCPKG_INSTALLED_PACKAGES} MATCHES ".*openexr${VCPKG_PLATFORM}.*")
    execute_process(COMMAND ${VCPKG_EXECUTABLE} install openexr${VCPKG_PLATFORM})
endif()

if (NOT ${VCPKG_INSTALLED_PACKAGES} MATCHES ".*sdl2${VCPKG_PLATFORM}.*")
    execute_process(COMMAND ${VCPKG_EXECUTABLE} install sdl2${VCPKG_PLATFORM})
endif()

if (NOT ${VCPKG_INSTALLED_PACKAGES} MATCHES ".*vulkan${VCPKG_PLATFORM}.*")
    execute_process(COMMAND ${VCPKG_EXECUTABLE} install vulkan${VCPKG_PLATFORM})
endif()

if (NOT ${VCPKG_INSTALLED_PACKAGES} MATCHES ".*stb${VCPKG_PLATFORM}.*")
    execute_process(COMMAND ${VCPKG_EXECUTABLE} install stb${VCPKG_PLATFORM})
endif()

execute_process(COMMAND ${VCPKG_EXECUTABLE} integrate install)
include(${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

find_package(assimp CONFIG REQUIRED)
find_package(OpenEXR CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(Vulkan REQUIRED)

set(BananLib banan_window.cpp banan_pipeline.cpp banan_device.cpp banan_logger.cpp banan_swap_chain.cpp banan_model.cpp banan_game_object.cpp banan_renderer.cpp banan_camera.cpp banan_buffer.cpp banan_descriptor.cpp banan_image.cpp)
set(BananTest Tests/BananEngineTest.cpp Tests/main.cpp Tests/Systems/PointLightSystem.cpp Tests/KeyboardMovementController.cpp Tests/Systems/ComputeSystem.cpp Tests/Systems/ProcrastinatedRenderSystem.cpp Tests/Systems/ResolveSystem.cpp Tests/Systems/PointShadowSystem.cpp Tests/Systems/PointShadowSystem.h)

add_library(BananEngine SHARED ${BananLib})
add_executable(BananEngineTest ${BananTest})
target_link_libraries(BananEngineTest PRIVATE BananEngine)
target_include_directories(BananEngineTest PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(BananEngine assimp::assimp OpenEXR::OpenEXR SDL2::SDL2main SDL2::SDL2 Vulkan::Vulkan)
include_directories(${ASSIMP_INCLUDE_DIRS} ${OpenEXR_INCLUDE_DIRS} ${Stb_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS})

