#include "vulkanImGui.h"
#include "vulkanTypes.h"
#include "vulkanCommandBuffer.h"

#include "memory/pmemory.h"

#include "systems/entitySystemComponent.h"

#include <external/glm/gtc/type_ptr.hpp>
#include <external/glm/gtx/quaternion.hpp>
#include "external/imgui/ImGuizmo.h"

struct imguiState
{
    VkDescriptorPool descriptorPool;
    const VulkanRenderpass* renderPass;
    const VulkanDevice* device;
    const VulkanSwapchain* swapchain;
};

static imguiState* imgui = nullptr;

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
    //ImGui::ShowDemoWindow();

    EntitySystem* entitySystem = EntitySystem::Get();
    auto& entities = entitySystem->getAvailableEntities();

    for(auto& it = entities.begin(); it != entities.end(); it++)
    {
        if(it->second[entitySystem->getComponentType<RenderComponent>(it->first)] == 1)
        {
            ImGui::PushID(it->first);
            if(ImGui::TreeNode("Mesh"))
            {
                static ImGuizmo::OPERATION currentOperation(ImGuizmo::TRANSLATE);
                static ImGuizmo::MODE currentMode(ImGuizmo::WORLD);

                CameraComponent* c = &entitySystem->getComponent<CameraComponent>(0);
                TransformComponent* t = &entitySystem->getComponent<TransformComponent>(it->first);

                ImGui::DragFloat3(" Position", &t->position.x, 1.0f);
                ImGui::DragFloat3(" Scale", &t->scale.x, 1.0f);
                if(ImGui::RadioButton("Translate", currentOperation == ImGuizmo::TRANSLATE))
                    currentOperation = ImGuizmo::TRANSLATE;
                ImGui::SameLine();
                if(ImGui::RadioButton("Rotate", currentOperation == ImGuizmo::ROTATE))
                    currentOperation = ImGuizmo::ROTATE;
                ImGui::SameLine();
                if(ImGui::RadioButton("Scale", currentOperation == ImGuizmo::SCALE))
                    currentOperation = ImGuizmo::SCALE;
                ImGui::SameLine();
                if(currentOperation != ImGuizmo::SCALE) {
                    if(ImGui::RadioButton("Local", currentMode == ImGuizmo::LOCAL))
                        currentMode = ImGuizmo::LOCAL;
                    ImGui::SameLine();
                    if(ImGui::RadioButton("World", currentMode == ImGuizmo::WORLD))
                        currentMode = ImGuizmo::WORLD;
                }

                glm::mat4 matrix = t->asMatrix();
                glm::mat4 cameraView = c->getView();
                f32 ratio = (f32)imgui->swapchain->extent.width / (f32)imgui->swapchain->extent.height;
                glm::mat4 projection = c->getProjection(ratio);

                ImGui::SameLine();
                if (ImGui::SmallButton("Reset"))
                {
                    if(currentOperation == ImGuizmo::TRANSLATE)
                        t->position = glm::vec3(0.0f);
                    else if(currentOperation == ImGuizmo::ROTATE)
                        t->rotation = glm::quat();
                    else if(currentOperation == ImGuizmo::SCALE)
                        t->scale = glm::vec3(1.0f);
                }

                ImGui::SameLine();
                if(ImGui::SmallButton("All"))
                {
                    t->position = glm::vec3(0.0f);
                    t->rotation = glm::quat();
                    t->scale = glm::vec3(1.0f);
                }

                ImGuizmo::BeginFrame();
                ImGuizmo::Manipulate(
                    glm::value_ptr(cameraView), 
                    glm::value_ptr(projection), 
                    currentOperation, 
                    currentMode, 
                    glm::value_ptr(matrix));

                if(ImGuizmo::IsUsing())
                {
                    f32 translation[3], rotation[3], scale[3];
                    ImGuizmo::DecomposeMatrixToComponents(
                        glm::value_ptr(matrix),
                        translation,
                        rotation,
                        scale);
                    
                    t->fromMatrix(matrix);
                }

                ImGuiIO& io = ImGui::GetIO();
                ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        if(it->second[entitySystem->getComponentType<LightPointComponent>(it->first)] == 1)
        {
            ImGui::PushID(it->first);
            if(ImGui::TreeNode("Light"))
            {
                LightPointComponent* comp = &entitySystem->getComponent<LightPointComponent>(it->first);
                ImGui::DragFloat3(" Position ", &comp->position.x, 1.0f);
                ImGui::DragFloat3(" Colour", &comp->color.r, 1.0f, 0.0f, 1.0f);
                ImGui::DragFloat(" Radius", &comp->radius, 0.1f, 0.0f);
                ImGui::DragFloat(" Intensity", &comp->intensity, 1.0f, 0.0f);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
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