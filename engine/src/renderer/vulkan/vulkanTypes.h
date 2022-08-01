#pragma once

#include "vulkan\vulkan.h"
#include "core\assert.h"
#include "core\logger.h"

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

struct VulkanRenderTarget
{
    /** @brief The vector of attachments hold in this render target. */
    std::vector<VkImageView> attachments;
    /** @brief The handle for the render target. */
    VkFramebuffer handle;
};

struct VulkanRenderpass
{
    /** @brief The clear color of the render pass. By default it's black.*/
    glm::vec4 clearColor = glm::vec4(0.0f);

    /** @brief The clear flags for this render pass. */
    u8 clearFlags;

    /** @brief The render target this render pass points to. */
    std::vector<VulkanRenderTarget> renderTargets;

    /** @brief The render pass handle.*/
    VkRenderPass handle;

    /** @brief The name of the previous pass.*/
    std::string previousPass;
    /** @brief The name of the next pass.*/
    std::string nextPass;
};

typedef struct CommandBuffer
{
    VkCommandBuffer handle;
} CommandBuffer;

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

/** @brief The Vertex data structure for Vulkan.*/
typedef struct VulkanVertex
{
    glm::vec3 position;
    glm::vec4 color;
    glm::vec2 uv;
    glm::vec3 normal;
}VulkanVertex;

/** @brief The Light data structure for Vulkan.*/
struct VulkanLightData
{
    glm::vec3 position;
    f32 intensity;
    glm::vec3 color;
    f32 radius;
    glm::vec3 forward;
    f32 cosineCutoff;
    f32 spotExponent;
    bool enabled;
    int type;
    f32 dummyValue;
};

/** @brief The global uniform buffer that will load to gpu every frame.*/
struct GlobalUniformBufferData
{
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 viewProjection;
    glm::mat4 inverseViewProjection;
    glm::vec3 position;
    f32 dummy;
};

typedef struct VulkanImage
{
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
} VulkanImage;

// TODO make configurable
/** @brief The maximum number of textures in the engine.*/
#define VULKAN_MAX_TEXTURES 512

typedef struct VulkanTexture
{
    VulkanImage image;
    VkSampler sampler;
} VulkanTexture;

// TODO make configurable
/** @brief The maximum number of meshes in the engine.*/
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

typedef struct VulkanShaderObject
{
    VkShaderModule shaderModule;
} VulkanShaderObject;

/** @brief The current maximum number of material instances in the engine.*/
#define VULKAN_MAX_MATERIAL_COUNT 512
/** @brief The number of descriptors per material instance. This should grow when addins textures.*/
#define VULKAN_MATERIAL_DESCRIPTOR_COUNT 1
/** @brief The number of sampler types. At the moment
 *  Diffuse, Normal and MetallicRoughness.*/
#define VULKAN_MATERIAL_SAMPLER_COUNT 3
/** @brief The maximum number of shader stages. Vertex and Fragment.*/
#define VULKAN_MAX_SHADER_STAGES 2

/** @brief Used to determine if a descriptor need updating or not.*/
struct VulkanDescriptorState
{
    u32 generations[3]; // one per frame
    u32 ids[3]; // Used for samplers
};

/** @brief Holding the descriptors (3 per each frame) for each material instance.*/
struct VulkanMaterialInstance
{
    /** @brief The id of the Material Instance state. */
    u32 id;
    /** @brief The offset to match into the uniform buffer.*/
    u32 offset;
    /** @brief Three descriptor sets per each frame per instance.*/
    VkDescriptorSet descriptorSets[3];
    /** @brief The state of the descriptor set.*/
    VulkanDescriptorState descriptorState[VULKAN_MATERIAL_DESCRIPTOR_COUNT];
};

/**
 * @note Due to  requirements from some GPU (NVidia I guess...) this
 * should be padded to 256 bytes.
 */
struct VulkanMaterialShaderUBO{
    glm::vec4 diffuseColor; // 16 bytes
    glm::vec4 reserved01;   // 16 bytes
    glm::vec4 reserved02;   // 16 bytes
    glm::vec4 reserved03;   // 16 bytes
    glm::mat4 matReserved01; // 64 bytes
    glm::mat4 matReserved02; // 64 bytes
    glm::mat4 matReserved03; // 64 bytes
};

/** @brief The Shader Stage configuration. */
struct VulkanShaderStageConfig
{
    /** @brief The name of the shader stage. */
    std::string name;
    /** @brief The Shader Stage Flags bits. */
    VkShaderStageFlagBits stage;
};

/** @brief The configuration for a Descriptor Set.*/
struct VulkanDescriptorSetConfig
{
    /** @brief A list of descriptor set layout bindings.*/
    std::vector<VkDescriptorSetLayoutBinding> vBindings;
    /** @brief The index of the sampler binding.*/
    u8 samplerBindingIndex;
};

/** @brief The Vulkan Pipeline Config given by the json file and generated in VulkanPipelineCreate() function. */
struct VulkanPipelineConfig
{
    /** @brief The pipeline name. */
    std::string name;

    /** @brief The List of shader stage configurations in this pipeline.*/
    std::vector<VulkanShaderStageConfig> vShaderStageConfigs;

    /** @brief Array of pool sizes. */
    VkDescriptorPoolSize poolSizes[2];

    /** @brief Total number of descriptor sets. 1 if only using globals, 2 otherwise. */
    u8 descriptorSetCount;
    /** @brief Descriptor sets, max of 2. Idx 0=global, 1=instance. */
    //std::vector<VulkanDescriptorSetConfig> vDescriptorSetConfigs;

    /** @brief A list of attribute description for this pipeline.*/
    std::vector<VkVertexInputAttributeDescription> vVertexDeclaration;
};

struct VulkanPipeline
{
    /** @brief The VulkanPipeline layout. */ 
    VkPipeline handle;
    /** @brief The VkPipeline handle. */
    VkPipelineLayout layout;
};

struct VulkanRenderPipeline
{
    /** @brief The data struct holding all information to properly create the pipeline.*/
    VulkanPipelineConfig pipelineConfig;

    /** @brief Shader stages, vertex and fragment at the moment.*/
    VulkanShaderObject shaderStages[VULKAN_MAX_SHADER_STAGES];

    /** @brief The descriptor pool used in this render pipeline. */
    VkDescriptorPool descriptorPool;

    /** @brief Descriptor set layouts, max of 2. Idx 0=global, 1=instance. */
    VkDescriptorSetLayout descriptorSetLayout[2];
    /** @brief One DescriptorSet per each buffer from the triple buffer. */
    VkDescriptorSet globalDescriptorSet[3];
    /** @brief The uniform buffer used by this shader. Globals + locals. */
    VulkanBuffer uniformBuffer;

    /** @brief The data holding the pipeline handle and layout.*/
    VulkanPipeline pipeline;

    /** @brief The renderpass to be used with this shader.*/
    VulkanRenderpass* renderpass;

    /*** @brief The push constant size. */
    u64 pushConstantSize;
    /**  @brief The push constant stride. */
    u64 pushConstantStride;

    /** @brief Texture uses. Idx 0=Difusse, 1=Normal, 2=MetallicRoughness. */
    TextureUse samplerUses [VULKAN_MATERIAL_SAMPLER_COUNT];
    /** @brief Instance states for all instances. */
    VulkanMaterialInstance materialInstances[VULKAN_MAX_MATERIAL_COUNT];

    /** @brief The minimum uniform buffer alignment given by the device properties.*/
    u64 uboAlignment;
    /** @brief The global UBO stride. */
    u64 globalUboStride;
    /** @brief The UBO stride. */
    u64 instanceUboStride;
};

/** @brief Vulkan Swapchain structure. */
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
} VulkanSwapchain;

/** @brief The vulkan structure holding all necessary data referred to the renderer.*/
typedef struct VulkanState
{
    /** @brief Handle to the Vulkan internal instance. */
    VkInstance      instance;
    /** @brief Handle to the Vulkan logical device. */
    VulkanDevice    device;

#if defined(DEBUG)
    /** @brief The debug messenger if app is in DEBUG mode. */
    VkDebugUtilsMessengerEXT debugMessenger;
#endif

    /** @brief The surface for the window to be drawn. */
    VkSurfaceKHR    surface;
    /** @brief The curren image index. */
    u32 imageIndex;
    /** @brief The actual index frame. */
    u32 currentFrame;

    VulkanSwapchainSupport swapchainSupport{};
    /** @brief The swapchain. */
    VulkanSwapchain swapchain;
    /** @brief Bool indicating if we are in the process of recreating. */
    bool recreatingSwapchain;

    /** @brief The framebuffer widht. */
    u32 clientWidth;
    /** @brief The framebuffer height. */
    u32 clientHeight;
    /** @brief Bool indicating if the window has been resized and we need to recreate the renderer. */
    bool resized;

    void* windowHandle;

    /** @brief The map holding a Pipeline with its name as a key. */
    std::map<std::string, VulkanRenderPipeline*> pipelines;
    /** @brief The render passe to be used in the pipeline. */
    std::map<std::string, VulkanRenderpass*> renderpasses;
    //std::map<std::string, VulkanRenderpass*> renderpasses;

    // TODO Temporal render pass.
    //VulkanRenderpass* renderpass;

    /** @brief Graphics command buffers, one per frame (3). */
    std::vector<CommandBuffer> commandBuffers;
    /** @brief Semaphores to indicate image availability, one per frame. */
    std::vector<VkSemaphore> imageAvailableSemaphores;
    /** @brief Semaphores to indicate queue availability, one per frame. */
    std::vector<VkSemaphore> renderFinishedSemaphores;
    /** @brief Fences to indicate when a frame is busy. */
    std::vector<VulkanFence> frameInFlightFences;
    /** @brief Array holding if the image is still in use. */
    VkFence imagesInFlight[3];

    // TODO temp ¿?
    // Store the current pipeline bound.
    VulkanRenderPipeline* boundPipeline = nullptr;

    // TODO implement own allocator for vulkan
    // VkAllocationCallbacks* allocator;

    // TODO Temporal variables
    VulkanMesh* vulkanMeshes;
} VulkanState;