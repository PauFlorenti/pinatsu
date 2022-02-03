#include "gltfLoader.h"

#include "core/logger.h"
#include "core/pstring.h"
#include "memory/pmemory.h"
#include "platform/filesystem.h"

#include "systems/meshSystem.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/tinygltf/tiny_gltf.h"

static Node*
loadNode(const tinygltf::Model& tmodel, const tinygltf::Node& tnode, Node* parent)
{
    Node* node = (Node*)memAllocate(sizeof(Node), MEMORY_TAG_ENTITY);

    // Load node's children
    if(tnode.children.size() > 0)
    {
        for( size_t i = 0; i < tnode.children.size(); ++i){
            loadNode(tmodel, tmodel.nodes[tnode.children[i]], node);
        }
    }

    if(tnode.mesh > -1)
    {
        const tinygltf::Mesh mesh = tmodel.meshes[tnode.mesh];

        MeshData meshData{};

        for(size_t i = 0; i < mesh.primitives.size(); ++i)
        {
            const tinygltf::Primitive& tprim = mesh.primitives[i];

            const f32* positionBuffer = nullptr;
            const f32* normalBuffer = nullptr;
            const f32* uvBuffer = nullptr;
            // Vertices
            {
                if(tprim.attributes.find("POSITION") != tprim.attributes.end())
                {
                    const tinygltf::Accessor& accessor = tmodel.accessors[tprim.attributes.find("POSITION")->second];
                    const tinygltf::BufferView& view = tmodel.bufferViews[accessor.bufferView];
                    positionBuffer = reinterpret_cast<const f32*>(&(tmodel.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    meshData.vertexCount = accessor.count;
                    meshData.vertices = (Vertex*)memAllocate(sizeof(Vertex) * meshData.vertexCount, MEMORY_TAG_ENTITY);
                }
            }

            for(size_t v = 0; v < meshData.vertexCount; ++v)
            {
                Vertex vert{};
                glm::vec3 pos   = glm::vec3(positionBuffer[v * 3], positionBuffer[v * 3 + 1], positionBuffer[v * 3 + 2]);
                vert.position   = glm::vec4(pos, 1.0f);
                vert.color      = glm::vec4(1);
                vert.uv         = glm::vec2(1);
                meshData.vertices[v] = vert;
            }
        }

        node->mesh = meshSystemCreateFromData(&meshData);
    }

    return node;
}

bool 
gltfLoaderLoad(ResourceLoader* self, const char* name, Resource* outResource)
{
    if(!self || !name || !outResource) {
        PERROR("gltfLoaderLoad - not enough information provided.");
        return false;
    }

    char fullPath[512];
    const char* format = "%s/%s/%s";
    stringFormat(fullPath, format, resourceSystemPath(), self->typePath, name);
    outResource->path = stringDuplicate(fullPath);

    FileHandle file;
    if(!filesystemOpen(fullPath, FILE_MODE_READ, false, &file)) {
        PERROR("gltfLoaderLoad - could not open file '%s'.", name);
        return false;
    }

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    std::string filename(fullPath);

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    //bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename);

    if(!warn.empty()) {
        PWARN("%s.", warn.c_str());
    }

    if(!err.empty()) {
        PERROR("%s.", err.c_str());
        return false;
    }

    if(!ret) {
        PFATAL("Failed to parse glTF '%s'.", name);
        return false;
    }

    Node* parent;

    for(const auto & scene : model.scenes)
    {
        for(const auto& nodeIdx : scene.nodes)
        {
            tinygltf::Node node = model.nodes[nodeIdx];

            parent = loadNode(model, node, nullptr);
        }
    }

    outResource->name = name;
    outResource->loaderId = self->id;
    outResource->dataSize = sizeof(Node*);
    outResource->data = parent;

    return true;
}

bool
gltfLoaderUnload(ResourceLoader* self, Resource* resource)
{
    return true;
}

ResourceLoader
gltfLoaderCreate()
{
    ResourceLoader loader;
    loader.id           = INVALID_ID;
    loader.load         = gltfLoaderLoad;
    loader.unload       = gltfLoaderUnload;
    loader.customType   = nullptr;
    loader.type         = RESOURCE_TYPE_GLTF;
    loader.typePath     = "meshes";
    return loader;
}