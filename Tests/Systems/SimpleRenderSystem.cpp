//
// Created by yashr on 12/4/21.
//

#include "SimpleRenderSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>

namespace Banan{

    struct SimplePushConstantData {
        glm::mat4 modelMatrix{1.f};
        glm::mat4 normalMatrix{1.f};
    };

    SimpleRenderSystem::SimpleRenderSystem(BananDevice &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> layouts) : bananDevice{device} {
        createPipelineLayout(layouts);
        createPipeline(renderPass);
    }

    SimpleRenderSystem::~SimpleRenderSystem() {
        vkDestroyPipelineLayout(bananDevice.device(), pipelineLayout, nullptr);
    }

    void SimpleRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> layouts) {

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

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

    void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "pipelineLayout must be created before pipeline");

        // why not use the existing variables HEIGHT or WIDTH? - On high pixel density displays such as apples "retina" display these values are incorrect, but the swap chain corrects these values
        PipelineConfigInfo pipelineConfig{};
        BananPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        bananPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/triangle.vert.spv", "shaders/triangle.frag.spv", pipelineConfig);
    }

    void SimpleRenderSystem::render(BananFrameInfo &frameInfo) {
        bananPipeline->bind(frameInfo.commandBuffer);
        std::vector<VkDescriptorSet> sets = {frameInfo.globalDescriptorSet, frameInfo.textureDescriptorSet, frameInfo.normalDescriptorSet, frameInfo.heightDescriptorSet};

        vkCmdBindDescriptorSets(frameInfo.commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelineLayout,0,sets.size(),sets.data(),0,nullptr);

        for (auto &kv : frameInfo.gameObjects) {
            auto &obj = kv.second;
            if (obj.model == nullptr) continue;

            vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(id_t), &kv.first);

            obj.model->bindAll(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }
}

