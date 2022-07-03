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

            ShadowSystem(BananDevice &device, VkDescriptorSetLayout globalSetLayout);

            void update(BananFrameInfo &frameInfo, GlobalUbo &ubo);
            void render(BananFrameInfo &frameInfo);

            VkDescriptorImageInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        private:
            void createShadowRenderPass();
            void createShadowDepthResources();
            void createShadowFramebuffers();

            void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
            void createPipeline();

            VkRenderPass renderPass;
            VkFormat frameBufferDepthFormat;
            VkFramebuffer frameBuffer;

            VkImage shadowDepthCubemapImage;
            VkDeviceMemory shadowDepthCubemapImageMemory;
            VkImageView shadowDepthCubemapImageView;
            VkSampler shadowDepthCubemapImageSampler;

            VkImage shadowDepthImage;
            VkDeviceMemory shadowDepthImageMemory;
            VkImageView shadowDepthImageView;

            VkImage shadowColorImage;
            VkDeviceMemory shadowColorImageMemory;
            VkImageView shadowColorImageView;

            BananDevice &bananDevice;
            std::unique_ptr<BananPipeline> bananPipeline;
            VkPipelineLayout pipelineLayout;
    };
}