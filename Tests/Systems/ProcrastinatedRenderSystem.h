//
// Created by yashr on 10/2/22.
//
#pragma once

#include <banan_camera.h>
#include <banan_pipeline.h>
#include <banan_device.h>
#include <banan_game_object.h>
#include <banan_frame_info.h>

#include <memory>
#include <vector>

namespace Banan{
    class ProcrastinatedRenderSystem {
    public:
        ProcrastinatedRenderSystem(const ProcrastinatedRenderSystem &) = delete;
        ProcrastinatedRenderSystem &operator=(const ProcrastinatedRenderSystem &) = delete;

        ProcrastinatedRenderSystem(BananDevice &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> layouts);
        ~ProcrastinatedRenderSystem();

        void render(BananFrameInfo &frameInfo);

    private:
        void createPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);
        void createPipeline(VkRenderPass renderPass);

        BananDevice &bananDevice;

        std::unique_ptr<BananPipeline> bananPipeline;
        VkPipelineLayout pipelineLayout;
    };
}
