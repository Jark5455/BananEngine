//
// Created by yashr on 12/19/22.
//

#include "ComputeSystem.h"

#include <stdexcept>

namespace Banan {
    ComputeSystem::ComputeSystem(BananDevice &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> layouts) : bananDevice{device} {
        createPipelineLayout(layouts);
        createPipelines(renderPass);
    }

    ComputeSystem::~ComputeSystem() {
        vkDestroyPipelineLayout(bananDevice.device(), pipelineLayout, nullptr);
    }

    void ComputeSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> layouts) {
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

    void ComputeSystem::createPipelines(VkRenderPass renderPass) {
        PipelineConfigInfo pipelineConfig{};
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        bananPipelines.push_back(std::make_shared<BananPipeline>(bananDevice, "shaders/calc_normal_mats.comp.spv", pipelineConfig));
    }

    void ComputeSystem::compute(BananFrameInfo &frameInfo) {
        for (auto pipeline : bananPipelines) {
            pipeline->bind(frameInfo.commandBuffer);
            vkCmdBindDescriptorSets(frameInfo.commandBuffer,VK_PIPELINE_BIND_POINT_COMPUTE,pipelineLayout,0,1,&frameInfo.globalDescriptorSet,0,nullptr);
            vkCmdDispatch(frameInfo.commandBuffer, (frameInfo.gameObjects.size() / 256) + 1, 1, 1);
        }
    }

    void ComputeSystem::reconstructPipeline(VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> layouts) {
        vkDestroyPipelineLayout(bananDevice.device(), pipelineLayout, nullptr);

        createPipelineLayout(layouts);
        createPipelines(renderPass);
    }
}
