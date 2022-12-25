//
// Created by yashr on 12/21/22.
//

#include "ProcrastinatedRenderSystem.h"

#include <stdexcept>

namespace Banan {
    ProcrastinatedRenderSystem::ProcrastinatedRenderSystem(BananDevice &device, VkRenderPass GBufferRenderPass, VkRenderPass mainRenderPass, std::vector<VkDescriptorSetLayout> layouts, std::vector<VkDescriptorSetLayout> procrastinatedLayouts) : bananDevice{device} {
        createGBufferPipelineLayout(layouts);
        createGBufferPipeline(GBufferRenderPass);

        createMainRenderTargetPipelineLayout(procrastinatedLayouts);
        createMainRenderTargetPipeline(mainRenderPass);
    }

    ProcrastinatedRenderSystem::~ProcrastinatedRenderSystem() {
        vkDestroyPipelineLayout(bananDevice.device(), GBufferPipelineLayout, nullptr);
        vkDestroyPipelineLayout(bananDevice.device(), mainRenderTargetPipelineLayout, nullptr);
    }

    void ProcrastinatedRenderSystem::calculateGBuffer(BananFrameInfo &frameInfo) {
        GBufferPipeline->bind(frameInfo.commandBuffer);
        std::vector<VkDescriptorSet> sets = {frameInfo.globalDescriptorSet, frameInfo.textureDescriptorSet, frameInfo.normalDescriptorSet, frameInfo.heightDescriptorSet};

        vkCmdBindDescriptorSets(frameInfo.commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,GBufferPipelineLayout,0,sets.size(),sets.data(),0,nullptr);

        for (auto &kv : frameInfo.gameObjects) {
            auto &obj = kv.second;
            if (obj.model == nullptr) continue;

            vkCmdPushConstants(frameInfo.commandBuffer, GBufferPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(id_t), &kv.first);

            obj.model->bindAll(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }

    void ProcrastinatedRenderSystem::render(BananFrameInfo &frameInfo) {
        mainRenderTargetPipeline->bind(frameInfo.commandBuffer);
        std::vector<VkDescriptorSet> sets = {frameInfo.globalDescriptorSet, frameInfo.procrastinatedDescriptorSet};

        vkCmdBindDescriptorSets(frameInfo.commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,mainRenderTargetPipelineLayout,0,sets.size(),sets.data(),0,nullptr);
        vkCmdDraw(frameInfo.commandBuffer, 3, 1, 0, 0);
    }

    void ProcrastinatedRenderSystem::createMainRenderTargetPipeline(VkRenderPass renderPass) {
        assert(mainRenderTargetPipelineLayout != nullptr && "pipelineLayout must be created before pipeline");

        // why not use the existing variables HEIGHT or WIDTH? - On high pixel density displays such as apples "retina" display these values are incorrect, but the swap chain corrects these values
        PipelineConfigInfo pipelineConfig{};
        BananPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = mainRenderTargetPipelineLayout;
        pipelineConfig.multisampleInfo.rasterizationSamples = bananDevice.getMaxUsableSampleCount();
        mainRenderTargetPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/mrt.vert.spv", "shaders/mrt.frag.spv", pipelineConfig);
    }

    void ProcrastinatedRenderSystem::createMainRenderTargetPipelineLayout(std::vector<VkDescriptorSetLayout> layouts) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(int);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &mainRenderTargetPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void ProcrastinatedRenderSystem::createGBufferPipeline(VkRenderPass renderPass) {
        assert(GBufferPipelineLayout != nullptr && "pipelineLayout must be created before pipeline");

        // why not use the existing variables HEIGHT or WIDTH? - On high pixel density displays such as apples "retina" display these values are incorrect, but the swap chain corrects these values
        PipelineConfigInfo pipelineConfig{};
        BananPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = GBufferPipelineLayout;
        pipelineConfig.multisampleInfo.rasterizationSamples = bananDevice.getMaxUsableSampleCount();
        GBufferPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/gbuffer.vert.spv", "shaders/gbuffer.frag.spv", pipelineConfig);

    }

    void ProcrastinatedRenderSystem::createGBufferPipelineLayout(std::vector<VkDescriptorSetLayout> layouts) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(int);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &GBufferPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }
}