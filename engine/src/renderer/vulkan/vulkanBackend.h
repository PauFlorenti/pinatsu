#pragma once

#include "renderer/renderTypes.h"
#include "vulkanTypes.h"

bool vulkanBackendInit(const char* appName, void* winHandle);
void vulkanBackendShutdown();

void vulkanBackendOnResize(u32 width, u32 height);

bool vulkanBeginFrame(f32 delta);
bool vulkanBeginRenderPass(const std::string& renderpass);
void vulkanForwardUpdateGlobalState(f32 dt);
void vulkanDrawGeometry(const RenderMeshData* mesh);
void vulkanEndRenderPass(const std::string& renderpass);
void vulkanEndFrame();
void vulkanImguiRender();

/** @brief Reads the pipelines.json configuration file and loads the specified pipelines.
 * @param none
*/
void vulkanLoadPipelines();

/** @brief Creates a render pass given some information via json.
 * @param const std::string& name
 * @param const json& j
 */
void vulkanCreateRenderPassFromJson(const std::string& name, const json& j);

bool vulkanCreateMesh(Mesh* mesh, u32 vertexCount, Vertex* vertices, u32 indexCount, u32* indices);
void vulkanDestroyMesh(const Mesh* mesh);
bool vulkanCreateTexture(void* data, Texture* texture);
void vulkanDestroyTexture(Texture* texture);
bool vulkanCreateMaterial(Material* m);

/**
 * @breif Receives a render pass configuration via json file and return a created renderpass.
 * @param[in, out] VulkanRenderpass Renderpass to be created.
 * @param[in] json config file.
 */
void vulkanRenderPassCreate(VulkanRenderpass* outRenderpass, const json& j);

/**
 * @brief Destroys the main render pass.
 * @param[in, out] VulkanRenderpass RenderPass to destroy
 */
void vulkanRenderPassDestroy(VulkanRenderpass* renderpass);

/**
 * @brief Create the graphics pipeline given the data stored in a json.
 * @param[in out] VulkanPipeline The pipeline result.
 * @param[in] string The name of the pipeline.
 * @param[in] json The file with the data configuration.
 */
bool vulkanCreatePipeline(VulkanPipeline* outPipeline, const std::string& name, const json& j);

/**
 * @brief Destroy the graphics pipeline.
 * @param VulkanPipeline The pipeline to destroy.
 */
void vulkanPipelineDestroy(VulkanPipeline* pipeline);

void vulkanUsePipeline(const std::string& name);

/**
 * @brief Binds the pipeline for use.
 * @param CommandBuffer Command buffer to assign the bind.
 * @param VkPipelineBindPoint Graphics pipeline bind point.
 * @param VulkanPipeline The pipeline to be bound.
 */
void vulkanBindPipeline(CommandBuffer* cmd, VkPipelineBindPoint bindPoint, VulkanPipeline* pipeline);

/**
 * @brief Binds the global buffer to the current bound pipeline.
 */
bool vulkanBindGlobals();

/**
 * @brief Applies the bound global buffer to be used by the shader.
 */
bool vulkanApplyGlobals();