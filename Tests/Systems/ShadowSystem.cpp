//
// Created by yashr on 6/29/22.
//

#include "ShadowSystem.h"

#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

namespace Banan {

    struct ShadowPushConstantData {
        glm::mat4 modelMatrix{1.f};
        glm::mat4 viewMatrix{1.f};
    };

    ShadowSystem::ShadowSystem(BananDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : bananDevice{device} {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    ShadowSystem::~ShadowSystem() {
        vkDestroyPipelineLayout(bananDevice.device(), pipelineLayout, nullptr);
    }

    void ShadowSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(ShadowPushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow pipeline layout!");
        }
    }

    void ShadowSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "pipelineLayout must be created before pipeline");

        PipelineConfigInfo pipelineConfig{};
        BananPipeline::shadowPipelineConfigInfo(pipelineConfig);
        pipelineConfig.attributeDescriptions = BananModel::Vertex::getAttributeDescriptions();
        pipelineConfig.bindingDescriptions = BananModel::Vertex::getBindingDescriptions();
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.renderPass = renderPass;

        bananPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/shadow.vert.spv", "shaders/shadow.frag.spv", pipelineConfig);
    }

    void ShadowSystem::render(BananFrameInfo &frameInfo, uint32_t faceindex) {

        switch (faceindex)
        {
            case 0: // POSITIVE_X
                frameInfo.shadowCubeMapCamera.setViewYXZ(frameInfo.gameObjects.at(2).transform.translation, {glm::radians(180.f), glm::radians(270.f), 0.f});
                break;
            case 1:	// NEGATIVE_X
                frameInfo.shadowCubeMapCamera.setViewYXZ(frameInfo.gameObjects.at(2).transform.translation, {glm::radians(180.f), glm::radians(90.f), 0.f});
                break;
            case 2:	// POSITIVE_Y
                frameInfo.shadowCubeMapCamera.setViewYXZ(frameInfo.gameObjects.at(2).transform.translation, {glm::radians(270.f), 0.f, 0.f});
                break;
            case 3:	// NEGATIVE_Y
                frameInfo.shadowCubeMapCamera.setViewYXZ(frameInfo.gameObjects.at(2).transform.translation, {glm::radians(90.f), 0.f, 0.f});
                break;
            case 5:	// POSITIVE_Z
                frameInfo.shadowCubeMapCamera.setViewYXZ(frameInfo.gameObjects.at(2).transform.translation, {0.f, 0.f, glm::radians(180.f)});
                break;
            case 4:	// NEGATIVE_Z
                frameInfo.shadowCubeMapCamera.setViewYXZ(frameInfo.gameObjects.at(2).transform.translation, {glm::radians(180.f), 0.f, 0.f});
                break;
        }

        bananPipeline->bind(frameInfo.commandBuffer);
        vkCmdBindDescriptorSets(frameInfo.commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelineLayout,0,1,&frameInfo.globalDescriptorSet,0,nullptr);



        for (auto &kv : frameInfo.gameObjects) {
            auto &obj = kv.second;
            if (obj.model == nullptr) continue;

            ShadowPushConstantData push{};
            push.modelMatrix = obj.transform.mat4();
            push.viewMatrix = frameInfo.shadowCubeMapCamera.getView();

            vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ShadowPushConstantData), &push);

            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }
}