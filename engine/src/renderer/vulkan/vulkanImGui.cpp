#include "vulkanImGui.h"
#include "vulkanTypes.h"
#include "vulkanCommandBuffer.h"

#include <external/imgui/imgui.h>
#include <external/imgui/imgui_impl_win32.h>
#include <external/imgui/imgui_impl_vulkan.h>
#include <external/imgui/ImGuizmo.h>
#include <external/glm/gtc/type_ptr.hpp>
#include <external/glm/gtx/quaternion.hpp>

#include "systems/entitySystemComponent.h"
#include "systems/components/comp_transform.h"
#include "systems/components/comp_name.h"
#include "systems/components/comp_light_point.h"
#include "systems/components/comp_parent.h"

#include "memory/pmemory.h"

struct imguiState
{
    VkDescriptorPool descriptorPool;
    const VulkanRenderpass* renderPass;
    const VulkanDevice* device;
    const VulkanSwapchain* swapchain;
};

static imguiState* imgui = nullptr;

static void
drawComponents(const Entity& ent);

void
imguiInit(
    VulkanState* state, 
    const VulkanRenderpass* renderpass)
{
    imgui = (imguiState*)memAllocate(sizeof(imguiState), MEMORY_TAG_MANAGER);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    imgui->device = &state->device;
    imgui->renderPass = renderpass;
    imgui->swapchain = &state->swapchain;

    VkDescriptorPoolSize desc{};
    desc.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    desc.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &desc;
    poolInfo.maxSets = 1;

    vkCreateDescriptorPool(imgui->device->handle, &poolInfo, nullptr, &imgui->descriptorPool);

    ImGui_ImplVulkan_InitInfo vkinit = {};
    vkinit.Instance = state->instance;
    vkinit.PhysicalDevice = imgui->device->physicalDevice;
    vkinit.Device = imgui->device->handle;
    vkinit.QueueFamily = imgui->device->graphicsQueueIndex;
    vkinit.Queue = imgui->device->graphicsQueue;
    vkinit.PipelineCache = nullptr;
    vkinit.DescriptorPool = imgui->descriptorPool;
    vkinit.MinImageCount = imgui->swapchain->imageCount;
    vkinit.ImageCount = imgui->swapchain->imageCount;

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((f32)state->clientWidth, (f32)state->clientHeight);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    ImGui_ImplWin32_Init(state->windowHandle);
    if(!ImGui_ImplVulkan_Init(&vkinit, imgui->renderPass->handle)){
        PERROR("Error initializing imgui.");
    }

    VkCommandBuffer cmd;
    vulkanCommandBufferAllocateAndBeginSingleUse(*imgui->device, imgui->device->commandPool, cmd);
    ImGui_ImplVulkan_CreateFontsTexture(cmd);
    vulkanCommandBufferEndSingleUse(*imgui->device, imgui->device->commandPool, imgui->device->graphicsQueue, cmd);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void
imguiRender(
    VkCommandBuffer& cmd,
    const RenderPacket& packet)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    EntitySystem* entitySystem = EntitySystem::Get();
    auto& entities = entitySystem->getAvailableEntities();

    std::vector<ComponentType> registeredComponentTypes = entitySystem->getAllComponentTypes();

    if(ImGui::TreeNode("Entities ..."))
    {
        for(auto& it = entities.begin(); it != entities.end(); it++)
        {
            const char* name = entitySystem->getComponent<TCompName>(it->first).getName();
            ImGui::PushID(it->first);
            if(ImGui::TreeNode(name))
            {
                drawComponents(it->first);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::TreePop();
    }
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}

void
imguiDestroy()
{
    vkDestroyDescriptorPool(imgui->device->handle, imgui->descriptorPool, nullptr);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

static void
drawComponents(const Entity& ent)
{
    EntitySystem* entitySystem = EntitySystem::Get();
    entitySystem->getComponent<TCompName>(ent).debugInMenu();
    entitySystem->getComponent<TCompTransform>(ent).debugInMenu();

    // The rest of components should be checked before writing in debug.
    if(entitySystem->hasComponent<TCompLightPoint>(ent))
    {
        entitySystem->getComponent<TCompLightPoint>(ent).debugInMenu();
    }

    if(entitySystem->hasComponent<TCompParent>(ent))
    {
        auto component = entitySystem->getComponent<TCompParent>(ent);
        if(component.parent == 0){
            component.debugInMenu();
        }
    }
}