//
// Created by yashr on 1/3/22.
//

#pragma once

#include "banan_window.h"
#include "banan_swap_chain.h"
#include "banan_shadow_mapper.h"
#include "banan_device.h"
#include "banan_model.h"
#include "banan_renderpass.h"

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
        VkRenderPass getGBufferRenderPass() const;
        float getAspectRatio() const;
        bool isFrameInProgress() const;
        VkCommandBuffer getCurrentCommandBuffer() const;

        int getFrameIndex() const;
        std::vector<VkDescriptorImageInfo> getShadowDescriptorInfo();
        std::vector<VkDescriptorImageInfo> getGBufferDescriptorInfo();

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void beginGBufferRenderPass(VkCommandBuffer commandBuffer);
        void endGBufferRenderPass(VkCommandBuffer commandBuffer);
        void beginShadowRenderPass(VkCommandBuffer commandBuffer);
        void endShadowRenderPass(VkCommandBuffer commandBuffer, uint32_t faceindex);

    private:
        void createCommandBuffers();
        void createShadowMapper();
        void freeCommandBuffers();
        void recreateSwapChain();
        void recreateGBufferRenderPass();

        BananWindow &bananWindow;
        BananDevice &bananDevice;
        std::unique_ptr<BananSwapChain> bananSwapChain;
        std::unique_ptr<BananShadowMapper> bananShadowMapper;
        std::unique_ptr<BananRenderPass> GBufferRenderPass;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted{false};
    };
}
