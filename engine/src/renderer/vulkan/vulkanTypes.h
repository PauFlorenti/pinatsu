#pragma once

#include "vulkan\vulkan.h"
#include "core\assert.h"
#include "core\logger.h"
#include "math_types.h"

#include <external\glm\glm.hpp>
#include <external\glm\gtc\matrix_transform.hpp>
#include <vector>

// TEMP
#include "resources/resourcesTypes.h"

#define VK_CHECK(x) { PASSERT(x == VK_SUCCESS); }

typedef struct VulkanDevice
{ 
    VkPhysicalDevice    physicalDevice;
    VkDevice            handle;

    u32 graphicsQueueIndex;
    u32 presentQueueIndex;
    u32 transferQueueIndex;
    u32 computeQueueIndex;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue computeQueue;
    VkQueue transferQueue;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    VkCommandPool commandPool;
    VkCommandPool transferCmdPool;

} VulkanDevice;

typedef struct VulkanSwapchainSupport
{
    VkSurfaceCapabilitiesKHR capabilities;
    u32 formatCount;
    VkSurfaceFormatKHR* formats;
    u32 presentModeCount;
    VkPresentModeKHR* presentModes;
} VulkanSwapchainSupport;

typedef struct VulkanRenderpass
{
    VkRenderPass handle;
} VulkanRenderpass;

typedef struct Framebuffer
{
    VkFramebuffer handle;
    std::vector<VkImageView> attachments;
    VulkanRenderpass* renderpass;
} Framebuffer;

typedef struct VulkanSwapchain
{
    VkSwapchainKHR      handle;
    VkSurfaceFormatKHR  format;
    VkPresentModeKHR    presentMode;
    VkExtent2D          extent;
    u32                 imageCount; // Number of images in the swapchain.
    u32                 maxImageInFlight; // Number of images in flight.

    std::vector<VkImage>        images;
    std::vector<VkImageView>    imageViews;
    std::vector<Framebuffer>    framebuffers;
} VulkanSwapchain;

typedef struct CommandBuffer
{
    VkCommandBuffer handle;
} CommandBuffer;

typedef struct VulkanPipeline
{
    VkPipelineLayout layout;
    VkPipeline pipeline;
} VulkanPipeline;

typedef struct VulkanFence
{
    VkFence handle;
    bool signaled;
} VulkanFance;

typedef struct VulkanBuffer
{
    VkBuffer handle;
    VkDeviceMemory memory;
} VulkanBuffer;

typedef struct VulkanVertex
{
    vec3 position;
    vec4 color;
    vec2 uv;
}VulkanVertex;

typedef struct ViewProjectionBuffer
{
    //glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
} ViewProjectionBuffer;

typedef struct VulkanImage
{
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
} VulkanImage;

// TODO make configurable
#define VULKAN_MAX_TEXTURES 10

typedef struct VulkanTexture
{
    VulkanImage image;
    VkSampler sampler;
} VulkanTexture;

// TODO make configurable
#define VULKAN_MAX_MESHES 10

typedef struct VulkanMesh
{
    u32 id;
    u32 vertexCount;
    u32 vertexSize;
    u32 vertexOffset;
    u32 indexCount;
    u32 indexOffset;
    VulkanBuffer vertexBuffer;    // TODO rethink this when ECS
    VulkanBuffer indexBuffer;
} VulkanMesh;

#define VULKAN_MAX_SHADER_STAGES 2

typedef struct VulkanShaderObject
{
    VkShaderModule shaderModule;
} VulkanShaderObject;

// Used to determine if a descriptor need updating or not.
typedef struct VulkanDescriptorState
{
    u32 generations[3]; // one per frame
    u32 ids[3]; // Used for samplers
} VulkanDescriptorState;

#define VULKAN_MAX_MATERIAL_COUNT 512

// At the moment just one for the Material info.
// May grow later for textures
#define VULKAN_FORWARD_MATERIAL_DESCRIPTOR_COUNT 2

// Holding the descriptors (3 per frame) for each material instance.
typedef struct VulkanMaterialInstance
{
    VkDescriptorSet descriptorSets[3];
    VulkanDescriptorState descriptorState[VULKAN_FORWARD_MATERIAL_DESCRIPTOR_COUNT];
} VulkanMaterialInstance;

/**
 * @note Due to  requirements from some GPU (NVidia I guess...) this
 * should be padded to 256 bytes.
 */
typedef struct VulkanMaterialShaderUBO{
    glm::vec4 diffuseColor; // 16 bytes
    glm::vec4 reserved01;   // 16 bytes
    glm::vec4 reserved02;   // 16 bytes
    glm::vec4 reserved03;   // 16 bytes
    glm::mat4 matReserved01; // 64 bytes
    glm::mat4 matReserved02; // 64 bytes
    glm::mat4 matReserved03; // 64 bytes
} VulkanMaterialShaderUBO;

/** Vulkan Material Shader
 * This object should hold all information related to
 * the shader pass.
 *  - Descriptors
 *  - Shader Stages
 */
typedef struct VulkanForwardShader
{
    // Vertex and fragment
    VulkanShaderObject shaderStages[VULKAN_MAX_SHADER_STAGES];

    // Global objects - Currently view and projection matrices.
    // global descriptors will be allocated from this pool
    VkDescriptorPool globalDescriptorPool;

     // One DescriptorSet per each buffer from the triple buffer
    VkDescriptorSet globalDescriptorSet[3];
    VkDescriptorSetLayout globalDescriptorSetLayout;

    // Struct to be loaded into global Ubo buffer.
    ViewProjectionBuffer globalUboData;
    // Buffer holding the data from globalUboData to be uploaded to the gpu.
    VulkanBuffer globalUbo;

    // Mesh instance objects
    VkDescriptorPool meshInstanceDescriptorPool;
    VkDescriptorSet meshInstanceDescriptorSet;
    VkDescriptorSetLayout meshInstanceDescriptorSetLayout;

    // This will grow
    VulkanMaterialShaderUBO objectMaterialData;
    u32 meshInstanceBufferIndex;
    VulkanBuffer meshInstanceBuffer;

    VulkanMaterialInstance materialInstances[VULKAN_MAX_MATERIAL_COUNT];

    VulkanPipeline pipeline;
} VulkanForwardShader;

typedef struct VulkanState
{
    VkInstance      instance;
    VulkanDevice    device;

    VkDebugUtilsMessengerEXT debugMessenger;

    VkSurfaceKHR    surface;

    u32 imageIndex; // Index to the swapchain image to paint.
    u32 currentFrame; // The actual index frame.

    // TODO implement own allocator for vulkan
    // VkAllocationCallbacks* allocator;

    // TODO Temporal variables
    Resource* aux;
    VulkanMesh* vulkanMeshes;

    // Forward rendering
    VulkanForwardShader forwardShader;

    VulkanSwapchainSupport swapchainSupport{};
    VulkanSwapchain swapchain;
    bool recreatingSwapchain;

    u32 clientWidth;
    u32 clientHeight;
    bool resized;

    VulkanRenderpass renderpass;

    std::vector<CommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VulkanFence> frameInFlightFences;
} VulkanState;