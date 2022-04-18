
# Find source files

set(SRC_CONTAINERS ${CMAKE_CURRENT_SOURCE_DIR}/src/containers/hashtable.cpp)

set(SRC_CORE ${CMAKE_CURRENT_SOURCE_DIR}/src/core/application.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/core/clock.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/core/event.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/core/input.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/core/logger.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/core/pstring.cpp)

set(SRC_MEMORY 
             ${CMAKE_CURRENT_SOURCE_DIR}/src/memory/linearAllocator.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/memory/pmemory.cpp)

set(SRC_PLATFORM 
             ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/filesystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/platform_win32.cpp)

set(SRC_RENDERER 
             ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/rendererBackend.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/rendererFrontend.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vulkan/vulkanBackend.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vulkan/vulkanBuffer.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vulkan/vulkanCommandBuffer.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vulkan/vulkanDevice.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vulkan/vulkanFramebuffer.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vulkan/vulkanImage.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vulkan/vulkanImGui.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vulkan/vulkanPipeline.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vulkan/vulkanRenderpass.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vulkan/vulkanShaderModule.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vulkan/vulkanSwapchain.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vulkan/vulkanUtils.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vulkan/vulkanVertexDeclaration.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vulkan/shaders/vulkanDeferredShader.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/vulkan/shaders/vulkanForwardShader.cpp)

set(SRC_RESOURCES 
             ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/loaders/gltfLoader.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/loaders/meshLoader.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/loaders/textureLoader.cpp)

set(SRC_EXTERNALS 
             ${CMAKE_CURRENT_SOURCE_DIR}/src/external/imgui/GraphEditor.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/external/imgui/ImCurveEdit.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/external/imgui/ImGradient.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/external/imgui/imgui_demo.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/external/imgui/imgui_draw.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/external/imgui/imgui_impl_vulkan.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/external/imgui/imgui_impl_win32.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/external/imgui/imgui_tables.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/external/imgui/imgui_widgets.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/external/imgui/imgui.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/external/imgui/ImGuizmo.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/external/imgui/ImSequencer.cpp)

set(SRC_SYSTEMS 
             ${CMAKE_CURRENT_SOURCE_DIR}/src/systems/entitySystemComponent.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/systems/materialSystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/systems/meshSystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/systems/physicsSystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/systems/resourceSystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/systems/textureSystem.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/systems/components/comp_light_point.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/systems/components/comp_name.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/systems/components/comp_parent.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/systems/components/comp_transform.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/systems/handle/handle.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/src/systems/handle/handleManager.cpp)

# Include necessary directories             
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${Vulkan_INCLUDE_DIRS})

# Warning compiler omissions
add_definitions(-wd4805)
add_definitions(-wd4267)
add_definitions(-wd4996)
add_definitions(-wd4244)
add_definitions(-wd4305)
add_definitions(-WX)
add_definitions(-Zi)
add_definitions(-DDEBUG)

# Generate Engine library
add_library(engine  STATIC ${SRC_CONTAINERS} 
                    ${SRC_CORE} 
                    ${SRC_EXTERNALS}
                    ${SRC_PLATFORM}
                    ${SRC_RENDERER} 
                    ${SRC_MEMORY} 
                    ${SRC_RESOURCES} 
                    ${SRC_SYSTEMS})