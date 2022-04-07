cmake_minimum_required(VERSION 3.0.0)
project(Pinatsu VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 11)

# Find Vulkan
set(Vulkan_INCLUDE_DIRECTORIES "${VULKAN_SDK_PATH}/Include")
set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib")

find_package(Vulkan REQUIRED)
message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")

if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan")
else()
    message(STATUS "Using Vulkan lib at: ${Vulkan_LIBRARIES}")
endif()

# Warning compiler omissions
add_definitions(-wd4805)
add_definitions(-wd4267)
add_definitions(-wd4996)
add_definitions(-wd4244)
add_definitions(-wd4305)
add_definitions(-WX)
add_definitions(-Zi)
add_definitions(-DDEBUG)

set(SRC_CONTAINERS ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/containers/hashtable.cpp)

set(SRC_CORE ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/core/application.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/core/clock.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/core/event.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/core/input.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/core/logger.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/core/pstring.cpp)

set(SRC_MEMORY 
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/memory/linearAllocator.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/memory/pmemory.cpp)

set(SRC_PLATFORM 
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/platform/filesystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/platform/platform_win32.cpp)

set(SRC_RENDERER 
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/renderer/rendererBackend.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/renderer/rendererFrontend.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/renderer/vulkan/vulkanBackend.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/renderer/vulkan/vulkanBuffer.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/renderer/vulkan/vulkanCommandBuffer.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/renderer/vulkan/vulkanDevice.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/renderer/vulkan/vulkanFramebuffer.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/renderer/vulkan/vulkanImage.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/renderer/vulkan/vulkanImGui.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/renderer/vulkan/vulkanPipeline.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/renderer/vulkan/vulkanRenderpass.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/renderer/vulkan/vulkanShaderModule.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/renderer/vulkan/vulkanSwapchain.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/renderer/vulkan/vulkanUtils.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/renderer/vulkan/vulkanVertexDeclaration.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/renderer/vulkan/shaders/vulkanDeferredShader.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/renderer/vulkan/shaders/vulkanForwardShader.cpp)

set(SRC_RESOURCES 
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/resources/loaders/gltfLoader.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/resources/loaders/meshLoader.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/resources/loaders/textureLoader.cpp)

set(SRC_EXTERNALS 
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/external/imgui/GraphEditor.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/external/imgui/ImCurveEdit.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/external/imgui/ImGradient.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/external/imgui/imgui_demo.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/external/imgui/imgui_draw.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/external/imgui/imgui_impl_vulkan.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/external/imgui/imgui_impl_win32.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/external/imgui/imgui_tables.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/external/imgui/imgui_widgets.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/external/imgui/imgui.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/external/imgui/ImGuizmo.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/external/imgui/ImSequencer.cpp)

set(SRC_SYSTEMS 
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/systems/entitySystemComponent.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/systems/materialSystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/systems/meshSystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/systems/physicsSystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/systems/resourceSystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/systems/textureSystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/systems/components/comp_light_point.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/systems/components/comp_name.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/engine/src/systems/components/comp_transform.cpp)

include_directories(${PROJECT_SOURCE_DIR}/engine/src)
include_directories(${Vulkan_INCLUDE_DIRS})

add_library(engine STATIC   ${SRC_CONTAINERS} 
                            ${SRC_CORE} 
                            ${SRC_RENDERER} 
                            ${SRC_PLATFORM}
                            ${SRC_MEMORY} 
                            ${SRC_RESOURCES} 
                            ${SRC_SYSTEMS}
                            ${SRC_EXTERNALS})

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/sandbox/src/entry.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/sandbox/src/sandbox.cpp)

include_directories(${PROJECT_SOURCE_DIR}/engine/src)
add_executable(sandbox ${SRC})
target_link_libraries(sandbox PUBLIC engine user32.lib ${Vulkan_LIBRARIES})


#add_subdirectory(engine)
#add_subdirectory(sandbox)
#
#target_link_libraries(Pinatsu PUBLIC engine)
#target_include_directories(Pinatsu PUBLIC
#                            "${PROJECT_SOURCE_DIR}/engine/engine/src")