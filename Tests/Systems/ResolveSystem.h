//
// Created by yashr on 2/3/23.
//

#pragma once

#include <banan_camera.h>
#include <banan_pipeline.h>
#include <banan_device.h>
#include <banan_game_object.h>
#include <banan_frame_info.h>

namespace Banan{
    class ResolveSystem {
    public:
        ResolveSystem(const ResolveSystem &) = delete;
        ResolveSystem &operator=(const ResolveSystem &) = delete;

        ResolveSystem(BananDevice &device, VkRenderPass edgeDetectionRenderPass, VkRenderPass blendWeightRenderPass, VkRenderPass resolveRenderPass, std::vector<VkDescriptorSetLayout> edgeDetectionLayouts, std::vector<VkDescriptorSetLayout> blendWeightLayouts, std::vector<VkDescriptorSetLayout> resolveLayouts);
        ~ResolveSystem();

        void runEdgeDetection(BananFrameInfo &frameInfo);
        void calculateBlendWeights(BananFrameInfo &frameInfo);
        void resolveImage(BananFrameInfo &frameInfo);

        void reconstructPipelines(VkRenderPass edgeDetectionRenderPass, VkRenderPass blendWeightRenderPass, VkRenderPass resolveRenderPass, std::vector<VkDescriptorSetLayout> edgeDetectionLayouts, std::vector<VkDescriptorSetLayout> blendWeightLayouts, std::vector<VkDescriptorSetLayout> resolveLayouts);

    private:
        void createEdgeDetectionPipeline(VkRenderPass renderPass);
        void createEdgeDetectionPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);

        void createBlendWeightPipeline(VkRenderPass renderPass);
        void createBlendWeightPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);

        void createResolvePipeline(VkRenderPass renderPass);
        void createResolvePipelineLayout(std::vector<VkDescriptorSetLayout> layouts);

        BananDevice &bananDevice;

        VkPipelineLayout edgeDetectionPipelineLayout;
        VkPipelineLayout blendWeightPipelineLayout;
        VkPipelineLayout resolvePipelineLayout;

        std::unique_ptr<BananPipeline> edgeDetectionPipeline;
        std::unique_ptr<BananPipeline> blendWeightPipeline;
        std::unique_ptr<BananPipeline> resolvePipeline;
    };
}
