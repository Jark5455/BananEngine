//
// Created by yashr on 10/2/22.
//

#include "ProcrastinatedRenderSystem.h"

#include <stdexcept>

namespace Banan {

    ProcrastinatedRenderSystem::ProcrastinatedRenderSystem(BananDevice &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> layouts) : bananDevice{device} {
        createPipelineLayout(layouts);
        createPipeline(renderPass);
    }

    ProcrastinatedRenderSystem::~ProcrastinatedRenderSystem() {
        vkDestroyPipelineLayout(bananDevice.device(), pipelineLayout, nullptr);
    }

    void ProcrastinatedRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> layouts) {

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void ProcrastinatedRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "pipelineLayout must be created before pipeline");

        PipelineConfigInfo pipelineConfig{};
        BananPipeline::procrastinatedPipelineConfigInfo(pipelineConfig);
        pipelineConfig.attributeDescriptions = {};
        pipelineConfig.bindingDescriptions = {};
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.renderPass = renderPass;

        bananPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/procrastinated.vert.spv", "shaders/procrastinated.frag.spv", pipelineConfig);
    }

    void ProcrastinatedRenderSystem::render(BananFrameInfo &frameInfo) {

    }
}