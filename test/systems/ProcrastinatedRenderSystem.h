//
// Created by yashr on 12/21/22.
//

#pragma once


#include <core/banan_camera.h>
#include <core/banan_pipeline.h>
#include <core/banan_device.h>
#include <core/banan_game_object.h>
#include <core/banan_frame_info.h>
#include <core/banan_renderpass.h>

#include <memory>
#include <vector>

using namespace Banan;

namespace BananTest {
    class ProcrastinatedRenderSystem {
    public:
        ProcrastinatedRenderSystem(const ProcrastinatedRenderSystem &) = delete;
        ProcrastinatedRenderSystem &operator=(const ProcrastinatedRenderSystem &) = delete;

        ProcrastinatedRenderSystem(BananDevice &device, VkRenderPass mainRenderPass, std::vector<VkDescriptorSetLayout> layouts, std::vector<VkDescriptorSetLayout> procrastinatedLayouts);
        ~ProcrastinatedRenderSystem();

        void calculateGBuffer(BananFrameInfo &frameInfo);
        void render(BananFrameInfo &frameInfo);

        void reconstructPipeline(VkRenderPass mainRenderPass, std::vector<VkDescriptorSetLayout> layouts, std::vector<VkDescriptorSetLayout> procrastinatedLayouts);

    private:
        void createMainRenderTargetPipeline(VkRenderPass renderPass);
        void createMainRenderTargetPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);

        void createGBufferPipeline(VkRenderPass renderPass);
        void createGBufferPipelineLayout(std::vector<VkDescriptorSetLayout> layouts);

        BananDevice &bananDevice;

        std::unique_ptr<BananRenderPass> GBufferRenderPass;
        std::unique_ptr<BananPipeline> GBufferPipeline;
        VkPipelineLayout GBufferPipelineLayout;

        std::unique_ptr<BananPipeline> mainRenderTargetPipeline;
        VkPipelineLayout mainRenderTargetPipelineLayout;
    };
}
