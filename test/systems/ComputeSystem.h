//
// Created by yashr on 12/19/22.
//

#pragma once

#include <banan_pipeline.h>
#include <banan_device.h>
#include <banan_frame_info.h>

using namespace Banan;

namespace BananTest {
    class ComputeSystem {
    public:
        ComputeSystem(const ComputeSystem &) = delete;
        ComputeSystem &operator=(const ComputeSystem &) = delete;

        ComputeSystem(BananDevice &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> layouts);
        ~ComputeSystem();

        void compute(BananFrameInfo &frameInfo);
        void reconstructPipeline(VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> layouts);

    private:
        void createPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);
        void createPipelines(VkRenderPass renderPass);

        BananDevice &bananDevice;
        std::vector<std::shared_ptr<BananPipeline>> bananPipelines;
        VkPipelineLayout pipelineLayout;
    };
}