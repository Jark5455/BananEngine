//
// Created by yashr on 2/3/23.
//

#include "ResolveSystem.h"

namespace Banan {

    ResolveSystem::ResolveSystem(BananDevice &device, VkRenderPass edgeDetectionRenderPass, VkRenderPass blendWeightRenderPass, VkRenderPass resolveRenderPass, std::vector<VkDescriptorSetLayout> edgeDetectionLayouts, std::vector<VkDescriptorSetLayout> blendWeightLayouts, std::vector<VkDescriptorSetLayout> resolveLayouts) : bananDevice{device} {
        createEdgeDetectionPipelineLayout(edgeDetectionLayouts);
        createEdgeDetectionPipeline(edgeDetectionRenderPass);

        createBlendWeightPipelineLayout(blendWeightLayouts);
        createBlendWeightPipeline(blendWeightRenderPass);

        createResolvePipelineLayout(resolveLayouts);
        createResolvePipeline(resolveRenderPass);
    }

    ResolveSystem::~ResolveSystem() {
        vkDestroyPipelineLayout(bananDevice.device(), edgeDetectionPipelineLayout, nullptr);
        vkDestroyPipelineLayout(bananDevice.device(), blendWeightPipelineLayout, nullptr);
        vkDestroyPipelineLayout(bananDevice.device(), resolvePipelineLayout, nullptr);
    }

    void ResolveSystem::createEdgeDetectionPipeline(VkRenderPass renderPass) {
        assert(edgeDetectionPipelineLayout != nullptr && "pipelineLayout must be created before pipeline");

        // why not use the existing variables HEIGHT or WIDTH? - On high pixel density displays such as apples "retina" display these values are incorrect, but the swap chain corrects these values
        PipelineConfigInfo pipelineConfig{};
        BananPipeline::defaultPipelineConfigInfo(pipelineConfig);

        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = edgeDetectionPipelineLayout;
        pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipelineConfig.subpass = 0;

        edgeDetectionPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/edge.vert.spv", "shaders/edge.frag.spv", pipelineConfig);
    }

    void ResolveSystem::createEdgeDetectionPipelineLayout(std::vector<VkDescriptorSetLayout> layouts) {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();

        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &edgeDetectionPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void ResolveSystem::createBlendWeightPipeline(VkRenderPass renderPass) {
        assert(blendWeightPipelineLayout != nullptr && "pipelineLayout must be created before pipeline");

        // why not use the existing variables HEIGHT or WIDTH? - On high pixel density displays such as apples "retina" display these values are incorrect, but the swap chain corrects these values
        PipelineConfigInfo pipelineConfig{};
        BananPipeline::defaultPipelineConfigInfo(pipelineConfig);

        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = blendWeightPipelineLayout;
        pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipelineConfig.subpass = 0;

        blendWeightPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/blend.vert.spv", "shaders/blend.frag.spv", pipelineConfig);
    }

    void ResolveSystem::createBlendWeightPipelineLayout(std::vector<VkDescriptorSetLayout> layouts) {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();

        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &blendWeightPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void ResolveSystem::createResolvePipeline(VkRenderPass renderPass) {
        assert(resolvePipelineLayout != nullptr && "pipelineLayout must be created before pipeline");

        // why not use the existing variables HEIGHT or WIDTH? - On high pixel density displays such as apples "retina" display these values are incorrect, but the swap chain corrects these values
        PipelineConfigInfo pipelineConfig{};
        BananPipeline::defaultPipelineConfigInfo(pipelineConfig);

        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();

        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = resolvePipelineLayout;
        pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipelineConfig.subpass = 0;

        resolvePipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/resolve.vert.spv", "shaders/resolve.frag.spv", pipelineConfig);
    }

    void ResolveSystem::createResolvePipelineLayout(std::vector<VkDescriptorSetLayout> layouts) {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();

        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &resolvePipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void ResolveSystem::reconstructPipelines(VkRenderPass edgeDetectionRenderPass, VkRenderPass blendWeightRenderPass, VkRenderPass resolveRenderPass, std::vector<VkDescriptorSetLayout> edgeDetectionLayouts, std::vector<VkDescriptorSetLayout> blendWeightLayouts, std::vector<VkDescriptorSetLayout> resolveLayouts) {
        vkDestroyPipelineLayout(bananDevice.device(), edgeDetectionPipelineLayout, nullptr);
        vkDestroyPipelineLayout(bananDevice.device(), blendWeightPipelineLayout, nullptr);
        vkDestroyPipelineLayout(bananDevice.device(), resolvePipelineLayout, nullptr);

        createEdgeDetectionPipelineLayout(edgeDetectionLayouts);
        createEdgeDetectionPipeline(edgeDetectionRenderPass);

        createBlendWeightPipelineLayout(blendWeightLayouts);
        createBlendWeightPipeline(blendWeightRenderPass);

        createResolvePipelineLayout(resolveLayouts);
        createResolvePipeline(resolveRenderPass);
    }

    void ResolveSystem::runEdgeDetection(BananFrameInfo &frameInfo) {
        edgeDetectionPipeline->bind(frameInfo.commandBuffer);
        std::vector<VkDescriptorSet> sets = {frameInfo.globalDescriptorSet, frameInfo.edgeDetectionDescriptorSet};

        vkCmdBindDescriptorSets(frameInfo.commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,edgeDetectionPipelineLayout,0,sets.size(),sets.data(),0,nullptr);
        vkCmdDraw(frameInfo.commandBuffer, 3, 1, 0, 0);
    }

    void ResolveSystem::calculateBlendWeights(BananFrameInfo &frameInfo) {
        blendWeightPipeline->bind(frameInfo.commandBuffer);
        std::vector<VkDescriptorSet> sets = {frameInfo.globalDescriptorSet, frameInfo.blendWeightDescriptorSet};

        vkCmdBindDescriptorSets(frameInfo.commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,blendWeightPipelineLayout,0,sets.size(),sets.data(),0,nullptr);
        vkCmdDraw(frameInfo.commandBuffer, 3, 1, 0, 0);
    }

    void ResolveSystem::resolveImage(BananFrameInfo &frameInfo) {
        resolvePipeline->bind(frameInfo.commandBuffer);
        std::vector<VkDescriptorSet> sets = {frameInfo.globalDescriptorSet, frameInfo.resolveDescriptorSet};

        vkCmdBindDescriptorSets(frameInfo.commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,resolvePipelineLayout,0,sets.size(),sets.data(),0,nullptr);
        vkCmdDraw(frameInfo.commandBuffer, 3, 1, 0, 0);
    }
}