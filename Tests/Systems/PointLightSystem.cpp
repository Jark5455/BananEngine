//
// Created by yashr on 12/4/21.
//

#include "PointLightSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <map>
#include <stdexcept>

namespace Banan{
    PointLightSystem::PointLightSystem(BananDevice &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> layouts) : bananDevice{device} {
        createPipelineLayout(layouts);
        createPipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem() {
        vkDestroyPipelineLayout(bananDevice.device(), pipelineLayout, nullptr);
    }

    void PointLightSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> layouts) {
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

    void PointLightSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "pipelineLayout must be created before pipeline");

        // why not use the existing variables HEIGHT or WIDTH? - On high pixel density displays such as apples "retina" display these values are incorrect, but the swap chain corrects these values
        PipelineConfigInfo pipelineConfig{};
        BananPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();
        BananPipeline::alphaBlendingPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipelineConfig.subpass = 2;
        bananPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/point_light.vert.spv", "shaders/point_light.frag.spv", pipelineConfig);
    }

    void PointLightSystem::render(BananFrameInfo &frameInfo) {
        std::map<float, BananGameObject::id_t> sorted;
        for (auto &kv : frameInfo.gameObjectManager.getGameObjects()) {
            auto &obj = kv.second;
            if (obj.pointLight == nullptr) continue;

            auto offset = frameInfo.camera.getPosition() - obj.transform.translation;
            float disSquared = glm::dot(offset, offset);
            sorted[disSquared] = obj.getId();
        }

        bananPipeline->bind(frameInfo.commandBuffer);

        std::vector<VkDescriptorSet> sets{frameInfo.globalDescriptorSet, frameInfo.gameObjectDescriptorSet};
        vkCmdBindDescriptorSets(frameInfo.commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelineLayout,0,sets.size(),sets.data(),0,nullptr);

        for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }

        vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
    }

    // temp func, just rotates lights
    void PointLightSystem::update(BananFrameInfo &frameInfo) {
        for (auto &kv : frameInfo.gameObjectManager.getGameObjects()) {
            auto &obj = kv.second;
            if (obj.pointLight == nullptr) continue;

            auto rotateLight = glm::rotate(glm::mat4(1.f), frameInfo.frameTime, {0.f, -1.f, 0.f});
            obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.f));
        }
    }

    void PointLightSystem::reconstructPipeline(VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> layouts) {
        vkDestroyPipelineLayout(bananDevice.device(), pipelineLayout, nullptr);

        createPipelineLayout(layouts);
        createPipeline(renderPass);
    }
}

