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
        private:

            void createOffscreenFramebuffers();
            void createOffscreenRenderpass();
            void createOffscreenImages();
            void recreateSwapChain();
            void createCommandBuffers();

            VkFramebuffer procrastinatedFramebuffer;
            VkRenderPass renderPass;

            std::unique_ptr<BananImage> positionImage;
            std::unique_ptr<BananImage> colorImage;
            std::unique_ptr<BananImage> normalImage;
            std::unique_ptr<BananImage> depthImage;

            BananDevice &bananDevice;
            BananWindow &bananWindow;
            std::unique_ptr<BananSwapChain> bananSwapChain;
            std::vector<VkCommandBuffer> commandBuffers;

            VkFormat depthFormat;
    };
}
