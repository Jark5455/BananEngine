//
// Created by yashr on 1/3/22.
//

#pragma once

#include "banan_window.h"
#include "banan_swap_chain.h"
#include "banan_shadow_mapper.h"
#include "banan_device.h"
#include "banan_model.h"

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
        VkRenderPass getShadowRenderPass() const;
        float getAspectRatio() const;
        bool isFrameInProgress() const;
        VkCommandBuffer getCurrentCommandBuffer() const;

        int getFrameIndex() const;
        VkDescriptorImageInfo getShadowDescriptorInfo();

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void beginShadowRenderPass(VkCommandBuffer commandBuffer);
        void endShadowRenderPass(VkCommandBuffer commandBuffer, uint32_t faceindex);

    private:
        void createCommandBuffers();
        void createShadowMapper();
        void freeCommandBuffers();
        void recreateSwapChain();

        BananWindow &bananWindow;
        BananDevice &bananDevice;
        std::unique_ptr<BananSwapChain> bananSwapChain;
        std::unique_ptr<BananShadowMapper> bananShadowMapper;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted{false};
    };
}
