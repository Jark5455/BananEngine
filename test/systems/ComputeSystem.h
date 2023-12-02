//
// Created by yashr on 12/19/22.
//

#pragma once

#include <core/banan_pipeline.h>
#include <core/banan_device.h>
#include <core/banan_frame_info.h>

namespace Banan {
    class ComputeSystem {
        public:
            ComputeSystem(const ComputeSystem &) = delete;
            ComputeSystem &operator=(const ComputeSystem &) = delete;

            ComputeSystem(BananDevice &device, std::vector<VkDescriptorSetLayout> layouts);
            ~ComputeSystem();

            void compute(BananFrameInfo &frameInfo);
            void reconstructPipeline(std::vector<VkDescriptorSetLayout> layouts);

        private:
            void createPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);
            void createPipelines();

            BananDevice &bananDevice;
            std::vector<std::shared_ptr<BananPipeline>> bananPipelines;
            VkPipelineLayout pipelineLayout;
        };
}
