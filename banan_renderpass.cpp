//
// Created by yashr on 12/21/22.
//

#include "banan_renderpass.h"

#include <utility>

namespace Banan {
    BananRenderPass::BananRenderPass(BananDevice &device, std::vector<VkFormat> attachmentFormats, VkExtent2D extent, bool depthImage) : bananDevice{device}, frameBufferFormats{attachmentFormats}, extent{extent} {
        if (depthImage)
            frameBufferFormats.push_back(device.findSupportedFormat({VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT));

        createFramebuffer(depthImage);
    }

    void BananRenderPass::createFramebuffer(bool depthImage) {
        for (VkFormat &format : frameBufferFormats) {
            frameBufferAttachments.push_back(std::make_shared<BananImage>(bananDevice, extent.width, extent.height, 1, format, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
        }
    }
}