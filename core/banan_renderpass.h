//
// Created by yashr on 12/21/22.
//

#include <memory>

#include "banan_device.h"
#include "banan_image.h"

#pragma once

namespace Banan {
    class BananRenderPass {
    public:
        BananRenderPass(BananDevice &device, std::vector<VkFormat> attachmentFormats, VkExtent2D extent, bool depthImage = true);
        ~BananRenderPass();

        void beginRenderPass(VkCommandBuffer commandBuffer, bool flippedViewport = false);
        void endRenderPass(VkCommandBuffer commandBuffer);

        VkRenderPass getRenderPass();

        std::vector<std::shared_ptr<BananImage>> getFramebufferAttachments();
    private:

        void createFramebuffer();
        void createRenderpass();

        VkRenderPass renderPass;
        VkFramebuffer frameBuffer;

        std::vector<std::shared_ptr<BananImage>> frameBufferAttachments;
        std::vector<VkFormat> frameBufferFormats;

        VkExtent2D extent;

        BananDevice &bananDevice;
    };

}
