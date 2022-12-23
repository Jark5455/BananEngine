//
// Created by yashr on 12/21/22.
//

#pragma once


#include <banan_camera.h>
#include <banan_pipeline.h>
#include <banan_device.h>
#include <banan_game_object.h>
#include <banan_frame_info.h>
#include <banan_renderpass.h>

#include <memory>
#include <vector>

namespace Banan{
    class ProcrastinatedRenderSystem {
    public:
        ProcrastinatedRenderSystem(const ProcrastinatedRenderSystem &) = delete;
        ProcrastinatedRenderSystem &operator=(const ProcrastinatedRenderSystem &) = delete;

        ProcrastinatedRenderSystem(BananDevice &device, VkRenderPass GBufferRenderPass, VkRenderPass mainRenderPass, std::vector<VkDescriptorSetLayout> layouts, std::vector<VkDescriptorSetLayout> procrastinatedLayouts);
        ~ProcrastinatedRenderSystem();

        void calculateGBuffer(BananFrameInfo &frameInfo);
        void render(BananFrameInfo &frameInfo);

    private:
        void createMainRenderTargetPipeline(std::vector<VkDescriptorSetLayout> layouts);
        void createMainRenderTargetPipelineLayout(VkRenderPass renderPass);

        void createGBufferPipeline(std::vector<VkDescriptorSetLayout> layouts);
        void createGBufferPipelineLayout(VkRenderPass renderPass);

        BananDevice &bananDevice;

        std::unique_ptr<BananRenderPass> GBufferRenderPass;
        std::unique_ptr<BananPipeline> GBufferPipeline;
        VkPipelineLayout GBufferPipelineLayout;

        std::unique_ptr<BananPipeline> mainRenderTargetPipeline;
        VkPipelineLayout mainRenderTargetPipelineLayout;

        std::vector<VkDescriptorSetLayout> layouts;
    };
}
