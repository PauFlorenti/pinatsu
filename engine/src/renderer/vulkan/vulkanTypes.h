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

#define MAX_LIGHTS 16

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
    vec3 normal;
}VulkanVertex;

struct VulkanLightData
{
    glm::vec3 position;
    f32 intensity;
    glm::vec3 color;
    f32 radius;
};

typedef struct ViewProjectionBuffer
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 position;
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
#define VULKAN_MAX_TEXTURES 512

typedef struct VulkanTexture
{
    VulkanImage image;
    VkSampler sampler;
} VulkanTexture;

// TODO make configurable
#define VULKAN_MAX_MESHES 512

typedef struct VulkanMesh
{
    u32 id;
    u32 vertexCount;
    u32 vertexSize;
    u32 vertexOffset;
    u32 indexCount;
    u32 indexOffset;
    VulkanBuffer vertexBuffer;
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
#define VULKAN_FORWARD_MATERIAL_DESCRIPTOR_COUNT 4

// At the moment it can hold samplers for diffuse, normal and MetallicRoughness
#define VULKAN_FORWARD_MATERIAL_SAMPLER_COUNT 3

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

    VulkanLightData lightData;
    VulkanBuffer lightUbo;

    // Mesh instance objects
    VkDescriptorPool meshInstanceDescriptorPool;
    VkDescriptorSet meshInstanceDescriptorSet;
    VkDescriptorSetLayout meshInstanceDescriptorSetLayout;

    // This will grow
    VulkanMaterialShaderUBO objectMaterialData;
    u32 meshInstanceBufferIndex;
    VulkanBuffer meshInstanceBuffer;

    TextureUse samplerUses [VULKAN_FORWARD_MATERIAL_SAMPLER_COUNT];

    VulkanMaterialInstance materialInstances[VULKAN_MAX_MATERIAL_COUNT];

    VulkanPipeline pipeline;
} VulkanForwardShader;

struct gbuffers
{
    VulkanTexture positonTxt;
    VulkanTexture normalTxt;
    VulkanTexture albedoTxt;
};

struct VulkanDeferredShader
{
    // 2 for geometry pass and 2 for deferred pass - maybe 2 more for postFX in the future.
    VulkanShaderObject shaderStages[4];
    
    // Geometry pass
    VkDescriptorPool geometryDescriptorPool;
    VkDescriptorPool lightDescriptorPool;
    // Per frame - descriptor global information
    VkDescriptorSet globalGeometryDescriptorSet;
    VkDescriptorSetLayout globalGeometryDescriptorSetLayout;
    // Per object - descriptor material object information
    u32 objectGeometryGeneration[VULKAN_MAX_MATERIAL_COUNT];
    u32 objectGeometryIds[VULKAN_MAX_MATERIAL_COUNT];
    VkDescriptorSet objectGeometryDescriptorSet[VULKAN_MAX_MATERIAL_COUNT];
    VkDescriptorSetLayout objectGeometryDescriptorSetLayout;
    
    ViewProjectionBuffer globalUboData;
    VulkanBuffer globalUbo;

    u32 objectBufferIndex = 0;
    VulkanBuffer objectUbo;

    TextureUse samplerUses [VULKAN_FORWARD_MATERIAL_SAMPLER_COUNT];

    // Deferred pass
    VkDescriptorSet lightDescriptorSet[3];
    VkDescriptorSetLayout lightDescriptorSetLayout;

    gbuffers gbuf;
    VulkanTexture geometryDepth;

    VkSemaphore geometrySemaphore;

    Framebuffer geometryFramebuffer;
    Framebuffer lightFramebuffer[3];
    VkCommandPool geometryCmdPool;
    CommandBuffer geometryCmdBuffer;

    VulkanPipeline geometryPipeline;
    VulkanPipeline lightPipeline;
    VulkanRenderpass geometryRenderpass;
    VulkanRenderpass lightRenderpass;
};

typedef struct VulkanSwapchain
{
    VkSwapchainKHR      handle;
    VkSurfaceFormatKHR  format;
    VkPresentModeKHR    presentMode;
    VkExtent2D          extent;
    u32                 imageCount; // Number of images in the swapchain.
    u32                 maxImageInFlight; // Number of images in flight.

    VkFormat depthFormat;
    VulkanImage depthImage;

    std::vector<VkImage>        images;
    std::vector<VkImageView>    imageViews;
    std::vector<Framebuffer>    framebuffers;
} VulkanSwapchain;

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
    VulkanMesh* vulkanMeshes;

    // Forward rendering
    VulkanForwardShader forwardShader;
    VulkanDeferredShader deferredShader;

    VulkanSwapchainSupport swapchainSupport{};
    VulkanSwapchain swapchain;
    bool recreatingSwapchain;

    u32 clientWidth;
    u32 clientHeight;
    bool resized;

    void* windowHandle;

    VulkanRenderpass renderpass;

    std::vector<CommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VulkanFence> frameInFlightFences;
} VulkanState;