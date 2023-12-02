//
// Created by yashr on 12/4/21.
//
#pragma once

#include <core/banan_camera.h>
#include <core/banan_pipeline.h>
#include <core/banan_device.h>
#include <core/banan_game_object.h>
#include <core/banan_frame_info.h>

#include <memory>
#include <vector>

namespace Banan{
    class PointLightSystem {
        public:
            PointLightSystem(const PointLightSystem &) = delete;
            PointLightSystem &operator=(const PointLightSystem &) = delete;

            PointLightSystem(BananDevice &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> layouts);
            ~PointLightSystem();

            void update(BananFrameInfo &frameInfo);
            void render(BananFrameInfo &frameInfo);

            void reconstructPipeline(VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> layouts);

    private:
            void createPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);
            void createPipeline(VkRenderPass renderPass);

            BananDevice &bananDevice;
            std::unique_ptr<BananPipeline> bananPipeline;
            VkPipelineLayout pipelineLayout;
    };
}
