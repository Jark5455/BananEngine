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
            using id_t = unsigned int;
            using Map = std::unordered_map<id_t, BananShadowMapper>;

            BananShadowMapper(BananDevice &device, id_t id);
            ~BananShadowMapper();

            BananShadowMapper(const BananShadowMapper &) = delete;
            BananShadowMapper &operator=(const BananShadowMapper &) = delete;
            BananShadowMapper(BananShadowMapper &&) = default;
            BananShadowMapper &operator=(BananShadowMapper &&) = default;

            VkDescriptorImageInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
            VkFramebuffer getFramebuffer();
            BananShadowMapper::id_t getId();

            void update(VkCommandBuffer commandBuffer, uint32_t faceindex);

            static void createShadowRenderPass(BananDevice &device);
            static VkRenderPass getRenderPass();

        private:
            void createShadowDepthResources();
            void createShadowFramebuffers();

            static VkRenderPass renderPass;
            static VkFormat frameBufferDepthFormat;
            VkFramebuffer frameBuffer;

            std::unique_ptr<BananCubemap> bananCubemap;
            std::unique_ptr<BananImage> bananDepthImage;
            std::unique_ptr<BananImage> bananColorImage;

            BananDevice &bananDevice;
            BananShadowMapper::id_t id;
    };
}
