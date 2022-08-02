#include "vulkanVertexDeclaration.h"

static VkVertexInputAttributeDescription 
layoutPosColorUVsNorm[] = 
{
    {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
    {1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(f32) * 3},
    {2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(f32) * 7},
    {3, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(f32) * 9}
};

VertexDeclaration vtx_decl_pos_color_uvs_norm("PosColorUvN", layoutPosColorUVsNorm, 4);

static VkVertexInputAttributeDescription
layoutPos[] = { {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0} };

VertexDeclaration vtx_decl_pos("Pos", layoutPos, 1);

static VkVertexInputAttributeDescription
layoutPosUV[] = {
    {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
    {1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(f32) * 3}
};

VertexDeclaration vtx_decl_pos_uv("PosUv", layoutPosUV, 2);

static VkVertexInputAttributeDescription
layoutPosColor[] = {
    {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
    {1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(f32) * 3}
};

VertexDeclaration vtx_decl_pos_color("PosColor", layoutPosColor, 2);

static VkVertexInputAttributeDescription
layoutPosColorUv[] = {
    {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
    {1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(f32) * 3},
    {2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(f32) * 7}
};

VertexDeclaration vtx_decl_pos_color_uvs("PosColorUv", layoutPosColorUv, 3);

VertexDeclaration::VertexDeclaration(const char* name, const VkVertexInputAttributeDescription* newLayout, u32 size)
    : name(name), layout(newLayout), size(size)
{ };


const VertexDeclaration*
getVertexDeclarationByName(const std::string& name)
{
    if(name == vtx_decl_pos.name)
        return &vtx_decl_pos;
    if (name == vtx_decl_pos_color.name)
        return &vtx_decl_pos_color;
    if(name == vtx_decl_pos_uv.name)
        return &vtx_decl_pos_uv;
    if(name == vtx_decl_pos_color_uvs_norm.name)
        return &vtx_decl_pos_color_uvs_norm;
    if(name == vtx_decl_pos_color_uvs.name)
        return &vtx_decl_pos_color_uvs;
    PWARN("No Vertex declaration found with such name %s", name.c_str());
    return nullptr;
}