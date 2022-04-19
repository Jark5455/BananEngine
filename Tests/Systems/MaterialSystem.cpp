//
// Created by yashr on 4/18/22.
//

#include "MaterialSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>


namespace Banan{

    struct MaterialPushConstantData {
        glm::vec4 diffuseColor;
        glm::vec4 specularColor;
        float specularShininess;
    };

    MaterialSystem::MaterialSystem(Banan::BananDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : bananDevice{device} {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    MaterialSystem::~MaterialSystem() {
        vkDestroyPipelineLayout(bananDevice.device(), pipelineLayout, nullptr);
    }

    void MaterialSystem::render(Banan::BananFrameInfo &frameInfo) {

    }

    void MaterialSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(MaterialSystem);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void MaterialSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "pipelineLayout must be created before pipeline");

        // why not use the existing variables HEIGHT or WIDTH? - On high pixel density displays such as apples "retina" display these values are incorrect, but the swap chain corrects these values
        PipelineConfigInfo pipelineConfig{};
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();
        BananPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        bananPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/material.vert.spv", "shaders/material.frag.spv", pipelineConfig);

    }
}
