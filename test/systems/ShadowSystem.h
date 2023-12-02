//
// Created by yashr on 6/29/22.
//

#pragma once

#include <core/banan_device.h>
#include <core/banan_pipeline.h>
#include <core/banan_frame_info.h>

namespace Banan {
    class ShadowSystem {
        public:
            ShadowSystem(const ShadowSystem &) = delete;
            ShadowSystem &operator=(const ShadowSystem &) = delete;

            ShadowSystem(BananDevice &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> layouts);
            ~ShadowSystem();

            void render(BananFrameInfo &frameInfo, uint32_t faceindex);

            void reconstructPipeline(VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> layouts);


    private:
            void createPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);
            void createPipeline(VkRenderPass renderPass);

            BananDevice &bananDevice;
            std::unique_ptr<BananPipeline> bananPipeline;
            VkPipelineLayout pipelineLayout;
    };
}