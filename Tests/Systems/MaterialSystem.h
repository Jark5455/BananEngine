//
// Created by yashr on 4/18/22.
//

#pragma once

#include "../../banan_camera.h"
#include "../../banan_pipeline.h"
#include "../../banan_device.h"
#include "../../banan_game_object.h"
#include "../../banan_frame_info.h"

#include <memory>
#include <vector>

namespace Banan {
    class MaterialSystem {
        public:
            MaterialSystem(const MaterialSystem &) = delete;
            MaterialSystem &operator=(const MaterialSystem &) = delete;

            MaterialSystem(BananDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
            ~MaterialSystem();

            void render(BananFrameInfo &frameInfo);
        private:
            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createPipeline(VkRenderPass renderPass);

            BananDevice &bananDevice;
            std::unique_ptr<BananPipeline> bananPipeline;
            VkPipelineLayout pipelineLayout;
    };
}
