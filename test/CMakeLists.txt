set(CMAKE_CXX_STANDARD 20)

add_executable(BananEngineTest main.cpp BananEngineTest.cpp systems/SimpleRenderSystem.cpp systems/PointLightSystem.cpp KeyboardMovementController.cpp systems/ComputeSystem.cpp systems/ProcrastinatedRenderSystem.cpp systems/ProcrastinatedRenderSystem.h)

target_link_libraries(BananEngineTest BananEngine)
target_link_libraries(BananEngineTest BananSTLExt)

include_directories(${CMAKE_SOURCE_DIR})

find_package(Vulkan REQUIRED)
if (NOT TARGET Vulkan::glslangValidator)
    message(FATAL_ERROR "GLSL Compiler not found")
endif()

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.comp"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${CMAKE_CURRENT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders/"
        COMMAND Vulkan::glslangValidator -V ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(shaders DEPENDS ${SPIRV_BINARY_FILES})

add_dependencies(BananEngineTest shaders)

add_custom_command(TARGET BananEngineTest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:BananEngineTest>/shaders/"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_BINARY_DIR}/shaders"
        "$<TARGET_FILE_DIR:BananEngineTest>/shaders"
)

file(COPY ${CMAKE_SOURCE_DIR}/test/banan_assets/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/banan_assets/)