#pragma once

#include "vulkanTypes.h"

/**
 * @brief Right now creates the main hardcoded renderpass.
 * @param VulkanState* pState
 * @return bool if succeded.
 */
bool vulkanRenderPassCreate(VulkanState* pState);

/**
 * @brief Destroys the main render pass.
 * @param VulkanState* pState
 * @return void
 */
void vulkanRenderPassDestroy(VulkanState* pState);