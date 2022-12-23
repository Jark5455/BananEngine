//
// Created by yashr on 12/21/22.
//

#include "ProcrastinatedRenderSystem.h"

#include <stdexcept>

namespace Banan {
    ProcrastinatedRenderSystem::ProcrastinatedRenderSystem(BananDevice &device, VkRenderPass GBufferRenderPass, VkRenderPass mainRenderPass, std::vector<VkDescriptorSetLayout> layouts, std::vector<VkDescriptorSetLayout> procrastinatedLayouts) : bananDevice{device}, layouts{layouts} {
    }

    ProcrastinatedRenderSystem::~ProcrastinatedRenderSystem() {

    }

    void ProcrastinatedRenderSystem::calculateGBuffer(BananFrameInfo &frameInfo) {

    }

    void ProcrastinatedRenderSystem::render(BananFrameInfo &frameInfo) {

    }

    void ProcrastinatedRenderSystem::createMainRenderTargetPipeline(std::vector<VkDescriptorSetLayout> layouts) {

    }

    void ProcrastinatedRenderSystem::createMainRenderTargetPipelineLayout(VkRenderPass renderPass) {

    }

    void ProcrastinatedRenderSystem::createGBufferPipeline(std::vector<VkDescriptorSetLayout> layouts) {

    }

    void ProcrastinatedRenderSystem::createGBufferPipelineLayout(VkRenderPass renderPass) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(int);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = (uint32_t) layouts.size();
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &GBufferPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }
}