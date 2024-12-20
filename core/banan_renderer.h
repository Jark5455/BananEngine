//
// Created by yashr on 1/3/22.
//

#pragma once

#include "banan_window.h"
#include "banan_swap_chain.h"
#include "banan_device.h"
#include "banan_model.h"
#include "banan_renderpass.h"

#include <volk.h>

#include <memory>
#include <vector>

namespace Banan{
    class BananRenderer {
    public:

        BananRenderer(const BananRenderer &) = delete;
        BananRenderer &operator=(const BananRenderer &) = delete;

        BananRenderer(BananWindow &window, BananDevice &device);
        ~BananRenderer();

        VkRenderPass getSwapChainRenderPass() const;
        float getAspectRatio() const;
        bool isFrameInProgress() const;
        VkCommandBuffer getCurrentCommandBuffer() const;

        int getFrameIndex() const;
        std::vector<VkDescriptorImageInfo> getGBufferDescriptorInfo();

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void recreateSwapChain();

    private:
        void createCommandBuffers();
        void freeCommandBuffers();

        BananWindow &bananWindow;
        BananDevice &bananDevice;
        std::unique_ptr<BananSwapChain> bananSwapChain;
        std::vector<VkCommandBuffer> commandBuffers{};
        std::vector<VkDescriptorImageInfo> GBufferInfo{};

        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted{false};
    };
}
