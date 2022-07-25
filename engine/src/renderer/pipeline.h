#pragma once

class CPipeline
{
    std::string name;

    u64 globalUboSize;
    u64 globalUboStride;

    u64 uboSize;
    u64 uboStride;

    u64 pushConstantSize;
    u64 pushConstantStride;

};