//
// Created by yashr on 12/4/21.
//
#pragma once

#include <banan_camera.h>
#include <banan_pipeline.h>
#include <banan_device.h>
#include <banan_game_object.h>
#include <banan_frame_info.h>

#include <memory>
#include <vector>

using namespace Banan;

namespace BananTest {
    class SimpleRenderSystem {
    public:
        SimpleRenderSystem(const SimpleRenderSystem &) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

        SimpleRenderSystem(BananDevice &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> layouts);
        ~SimpleRenderSystem();

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
