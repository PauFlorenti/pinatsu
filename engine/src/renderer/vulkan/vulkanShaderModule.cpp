#include "vulkanShaderModule.h"

#include <fstream>

// TODO create a function to read files and treat shaders accordingly.
bool 
readShaderFile(const char* filename, std::vector<char>& buffer)
{
    std::ifstream file(filename, std::ifstream::ate | std::ifstream::binary);

    if(!file.is_open()){
        PERROR("File %s could not be opened!", filename);
        return false;
    }

    // Check if file has length and if it is multiple of 4.
    // To be a valid binary file to be passed to the shader it must be multiple of 4.
    size_t length = file.tellg();
    if(length == 0 || length % 4 != 0) {
        PERROR("File does not meet requirements.");
        return false;
    }
    buffer.resize(length);
    file.seekg(0);
    file.read(buffer.data(), length);
    file.close();
    return true;
};

/**
 * @brief Creates a shader module given a valid buffer with the
 * shader binary data in it.
 * @param VulkanState* pState,
 * @param std::vector<char>& buffer conaining all spvr binary data
 * @param VkShaderModule* The shader module to be created.
 * @return void
 */
void vulkanCreateShaderModule(
    VulkanState* pState,
    std::vector<char>& buffer,
    VkShaderModule* module)
{
    VkShaderModuleCreateInfo info = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    info.codeSize   = static_cast<u32>(buffer.size());
    info.pCode      = reinterpret_cast<const u32*>(buffer.data());
    
    VK_CHECK(vkCreateShaderModule(
        pState->device.handle, 
        &info, 
        nullptr, 
        module));
};