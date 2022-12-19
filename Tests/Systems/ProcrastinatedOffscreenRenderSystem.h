//
// Created by yashr on 10/4/22.
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
    class ProcrastinatedOffscreenRenderSystem {
    public:
        ProcrastinatedOffscreenRenderSystem(const ProcrastinatedOffscreenRenderSystem &) = delete;
        ProcrastinatedOffscreenRenderSystem &operator=(const ProcrastinatedOffscreenRenderSystem &) = delete;

        ProcrastinatedOffscreenRenderSystem(BananDevice &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> layouts);
        ~ProcrastinatedOffscreenRenderSystem();

        void render(BananFrameInfo &frameInfo);

    private:
        void createPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);
        void createPipeline(VkRenderPass renderPass);

        BananDevice &bananDevice;

        std::unique_ptr<BananPipeline> bananPipeline;
        VkPipelineLayout pipelineLayout;
    };
}
