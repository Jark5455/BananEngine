set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

# TODO eventually phase out the following dependencies
find_package(assimp REQUIRED)
find_package(stb REQUIRED)
find_package(OpenEXR REQUIRED)
find_package(libdeflate REQUIRED)
find_package(ZLIB REQUIRED)

add_library(BananEngine SHARED banan_window.cpp banan_pipeline.cpp banan_device.cpp banan_swap_chain.cpp banan_model.cpp banan_game_object.cpp banan_renderer.cpp banan_camera.cpp banan_buffer.cpp banan_descriptor.cpp banan_image.cpp banan_renderpass.cpp)

target_link_libraries(BananEngine SDL2::SDL2main)

if(TARGET SDL2::SDL2-static)
    target_link_libraries(BananEngine SDL2::SDL2-static)
else()
    target_link_libraries(BananEngine SDL2::SDL2)
endif()

if(TARGET libdeflate::libdeflate_static)
    target_link_libraries(BananEngine libdeflate::libdeflate_static)
else()
    target_link_libraries(BananEngine libdeflate::libdeflate_shared)
endif()

target_link_libraries(BananEngine ZLIB::ZLIB)
target_link_libraries(BananEngine assimp::assimp)
target_link_libraries(BananEngine stb::stb)
target_link_libraries(BananEngine OpenEXR::OpenEXR)
target_link_libraries(BananEngine Imath::Imath)
target_link_libraries(BananEngine Vulkan::Vulkan)

include_directories(${Vulkan_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})

# TODO eventually phase out the following dependencies
include_directories(${assimp_INCLUDE_DIRS} ${OpenEXR_INCLUDE_DIRS} ${Imath_INCLUDE_DIRS} ${stb_INCLUDE_DIRS})