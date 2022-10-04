//
// Created by yashr on 10/4/22.
//

#include "ProcrastinatedOffscreenRenderSystem.h"

#include <stdexcept>

namespace Banan {

    struct ProcrastinatedOffscreenPushConstantData {
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
    };

    ProcrastinatedOffscreenRenderSystem::ProcrastinatedOffscreenRenderSystem(BananDevice &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> layouts) : bananDevice{device} {
        createPipelineLayout(layouts);
        createPipeline(renderPass);
    }

    ProcrastinatedOffscreenRenderSystem::~ProcrastinatedOffscreenRenderSystem() {
        vkDestroyPipelineLayout(bananDevice.device(), pipelineLayout, nullptr);
    }

    void ProcrastinatedOffscreenRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> layouts) {

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(ProcrastinatedOffscreenPushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void ProcrastinatedOffscreenRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "pipelineLayout must be created before pipeline");

        VkPipelineColorBlendAttachmentState state{};
        state.blendEnable = VK_FALSE;
        state.colorWriteMask = 0xf;

        std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates = {3, state};

        PipelineConfigInfo pipelineConfig{};
        BananPipeline::procrastinatedPipelineConfigInfo(pipelineConfig);
        pipelineConfig.colorBlendInfo.attachmentCount = 3;
        pipelineConfig.colorBlendInfo.pAttachments = blendAttachmentStates.data();
        pipelineConfig.attributeDescriptions = BananModel::Vertex::getAttributeDescriptions();
        pipelineConfig.bindingDescriptions = BananModel::Vertex::getBindingDescriptions();
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.renderPass = renderPass;

        bananPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/mrt.vert.spv", "shaders/mrt.frag.spv", pipelineConfig);
    }

    void ProcrastinatedOffscreenRenderSystem::render(BananFrameInfo &frameInfo) {

    }
}