//
// Created by yashr on 6/29/22.
//

#pragma once

#include "../../banan_device.h"
#include "../../banan_pipeline.h"
#include "../../banan_frame_info.h"

namespace Banan {
    class ShadowSystem {
        public:
            ShadowSystem(const ShadowSystem &) = delete;
            ShadowSystem &operator=(const ShadowSystem &) = delete;

            ShadowSystem(BananDevice &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> layouts);
            ~ShadowSystem();

            void render(BananFrameInfo &frameInfo, uint32_t faceindex);

        private:
            void createPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);
            void createPipeline(VkRenderPass renderPass);

            BananDevice &bananDevice;
            std::unique_ptr<BananPipeline> bananPipeline;
            VkPipelineLayout pipelineLayout;
    };
}