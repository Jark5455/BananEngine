//
// Created by yashr on 10/2/22.
//

#pragma once

#include "banan_device.h"
#include "banan_image.h"
#include "banan_swap_chain.h"

#include <memory>
#include <vector>

namespace Banan {
    class BananProcrastinatedRenderer {
        public:
            BananProcrastinatedRenderer(BananWindow &window, BananDevice &device);
            ~BananProcrastinatedRenderer();

            BananProcrastinatedRenderer(const BananProcrastinatedRenderer &) = delete;
            BananProcrastinatedRenderer &operator=(const BananProcrastinatedRenderer &) = delete;

            VkRenderPass getSwapchainRenderpass();
            VkRenderPass getOffscreenRenderPass();

            float getAspectRatio();
            bool isFrameInProgress() const;
            VkCommandBuffer getCurrentCommandBuffer();

            int getCurrentFrameIndex();
            VkCommandBuffer beginFrame();
            void endFrame();
            void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
            void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
            void beginProcrastinatedRenderpass(VkCommandBuffer commandBuffer);
            void endProcrastinatedRenderpass(VkCommandBuffer commandBuffer);

        private:

            void createProcrastinatedFramebuffers();
            void createProcrastinatedRenderpass();
            void createProcrastinatedImages();
            void createProcrastinatedCommandBuffer();
            void recreateSwapChain();
            void createCommandBuffers();

            VkFramebuffer procrastinatedFramebuffer;
            VkRenderPass procrastinatedRenderPass;
            VkCommandBuffer procrastinatedCommandBuffer;
            VkSemaphore procrastinatedSemaphore;

            std::unique_ptr<BananImage> positionImage;
            std::unique_ptr<BananImage> colorImage;
            std::unique_ptr<BananImage> normalImage;
            std::unique_ptr<BananImage> depthImage;

            BananDevice &bananDevice;
            BananWindow &bananWindow;
            std::unique_ptr<BananSwapChain> bananSwapChain;
            std::vector<VkCommandBuffer> swapchainCommandBuffers;

            VkFormat depthFormat;
            uint32_t currentImageIndex;
            int currentFrameIndex{0};
            bool isFrameStarted{false};
    };
}
