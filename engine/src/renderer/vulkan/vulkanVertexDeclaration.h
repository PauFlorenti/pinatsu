#pragma once

#include "vulkan\vulkan.h"

struct VertexDeclaration
{
    u32 size = 0;
    const VkVertexInputAttributeDescription* layout = nullptr;
    std::string name;
    
    VertexDeclaration(const char* name, const VkVertexInputAttributeDescription* newLayout, u32 size);
};

const VertexDeclaration*
getVertexDeclarationByName(const std::string& name);