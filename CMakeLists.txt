# CMakeList.txt : CMake project for LittleMayaEngine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Versioning
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 38)
set(PROJECT_VERSION_PATCH 9)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Project
project("LittleMayaEngine" VERSION ${PROJECT_VERSION})

set(GLSLANG_VALIDATOR "C:/VulkanSDK/1.3.250.1/Bin/glslangValidator.exe")

function(compile_shader SRC_FILE OUT_FILE)
    set(SHADER_SOURCE "${CMAKE_SOURCE_DIR}/${SRC_FILE}")
    set(SHADERS_DIR "${CMAKE_BINARY_DIR}/shaders")
    set(SPIRV_BINARY "${SHADERS_DIR}/${OUT_FILE}")
    
    file(MAKE_DIRECTORY ${SHADERS_DIR})
    
    add_custom_command(
        OUTPUT ${SPIRV_BINARY}
        COMMAND ${GLSLANG_VALIDATOR} -V ${SHADER_SOURCE} -o ${SPIRV_BINARY}
        DEPENDS ${SHADER_SOURCE})
    set_source_files_properties(${SPIRV_BINARY} PROPERTIES GENERATED TRUE)
    target_sources(LittleMayaEngine PRIVATE ${SPIRV_BINARY})
endfunction(compile_shader)

# Add source to this project's executable.
add_executable (LittleMayaEngine
"main.cpp"
"core/Logger.h" "core/Logger.cpp"
"core/App.h" "core/App.cpp"
"core/Window.h" "core/Window.cpp"
"ecs/GameObject.h" "ecs/GameObject.cpp"
"render/Device.h" "render/Device.cpp"
"render/Model.h" "render/Model.cpp"
"render/Pipeline.h" "render/Pipeline.cpp"
"render/Renderer.h" "render/Renderer.cpp"
"render/SwapChain.h" "render/SwapChain.cpp"
"render/Camera.h" "render/Camera.cpp"
"core/KeyboardMovementController.h" "core/KeyboardMovementController.cpp"
"core/Utils.h"
"render/Buffer.h" "render/Buffer.cpp"
"render/FrameInfo.h"
"render/Descriptors.h" "render/Descriptors.cpp"
"systems/PointLightSystem.h" "systems/PointLightSystem.cpp"
"systems/RenderSystem.h" "systems/RenderSystem.cpp"
"systems/EventSystem.h" "systems/EventSystem.cpp")

target_compile_definitions(LittleMayaEngine PRIVATE MODEL_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/models/")

# shaders
compile_shader("shaders/shader.vert" "shader.vert.spv")
compile_shader("shaders/shader.frag" "shader.frag.spv")
compile_shader("shaders/point_light.vert" "point_light.vert.spv")
compile_shader("shaders/point_light.frag" "point_light.frag.spv")

# spdlog
add_subdirectory ("C:/source/repos/LittleMayaEngine/libs/spdlog")
target_link_libraries(LittleMayaEngine PRIVATE spdlog)
target_include_directories(LittleMayaEngine PRIVATE "C:/source/repos/LittleMayaEngine/libs/spdlog/include")

# Vulkan
set(VULKAN_SDK_PATH "C:/VulkanSDK/1.3.250.1/Include" CACHE PATH "Path to Vulkan SDK")
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
target_link_libraries(LittleMayaEngine PRIVATE ${Vulkan_LIBRARIES})

# Vulkan Memory Allocator (VMA)
set(VMA_INCLUDE_DIR "C:/VulkanMemoryAllocator/include" CACHE PATH "Path to VMA include directory")
include_directories(${VMA_INCLUDE_DIR})

# glm
set(GLM_INCLUDE_DIR "C:/source/repos/LittleMayaEngine/libs/glm" CACHE PATH "Path to GLM include directory")
include_directories(${GLM_INCLUDE_DIR})

# GLFW3
set(GLFW_INCLUDE_DIR "C:/source/repos/LittleMayaEngine/libs/glfw-3.3.8.bin.WIN64/include" CACHE PATH "Path to GLFW include directory")
set(GLFW_LIBRARY_DIR "C:/source/repos/LittleMayaEngine/libs/glfw-3.3.8.bin.WIN64/lib-vc2022" CACHE PATH "Path to GLFW library directory")
include_directories(${GLFW_INCLUDE_DIR})
target_link_directories(LittleMayaEngine PRIVATE ${GLFW_LIBRARY_DIR})
target_link_libraries(LittleMayaEngine PRIVATE glfw3)

# Assimp
set(ASSIMP_INCLUDE_DIR "C:/source/repos/LittleMayaEngine/libs/assimp/include" CACHE PATH "Path to Assimp include directory")
set(ASSIMP_LIBRARY_DIR "C:/source/repos/LittleMayaEngine/libs/assimp/lib" CACHE PATH "Path to Assimp library directory")
set(ASSIMP_LIBRARIES
    "${ASSIMP_LIBRARY_DIR}/Debug/assimp-vc143-mtd.lib"
)
include_directories(${ASSIMP_INCLUDE_DIR})
target_link_libraries(LittleMayaEngine PRIVATE ${ASSIMP_LIBRARIES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET LittleMayaEngine PROPERTY CXX_STANDARD 20)
endif()

# Specify compile options for the target.
if(MSVC)
    target_compile_options(LittleMayaEngine PRIVATE /wd4820)
endif()

set_target_properties(LittleMayaEngine PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/out/build/x64-debug/")

# TODO: Add tests and install targets if needed.
