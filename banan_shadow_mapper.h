//
// Created by yashr on 7/2/22.
//

#pragma once

#include "banan_device.h"
#include "banan_image.h"
#include "banan_pipeline.h"
#include "banan_frame_info.h"

namespace Banan {
    class BananShadowMapper {
    public:
        BananShadowMapper(const BananShadowMapper &) = delete;
        BananShadowMapper &operator=(const BananShadowMapper &) = delete;

        BananShadowMapper(BananDevice &device);
        ~BananShadowMapper();

        VkDescriptorImageInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkRenderPass getRenderPass();
        VkFramebuffer getFramebuffer();

        void update(VkCommandBuffer commandBuffer, uint32_t faceindex);

    private:
        void createShadowRenderPass();
        void createShadowDepthResources();
        void createShadowFramebuffers();

        VkRenderPass renderPass;
        VkFormat frameBufferDepthFormat;
        VkFramebuffer frameBuffer;

        std::unique_ptr<BananCubemap> bananCubemap;
        std::unique_ptr<BananImage> bananDepthImage;
        std::unique_ptr<BananImage> bananColorImage;

        BananDevice &bananDevice;
    };
}
