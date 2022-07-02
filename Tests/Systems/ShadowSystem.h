//
// Created by yashr on 6/29/22.
//

#pragma once

#include "../../banan_device.h"

namespace Banan {
    class ShadowSystem {
        public:

        private:
            void createShadowRenderPass();
            void createShadowDepthResources();
            void createShadowFramebuffers();

            VkImage shadowDepthImage;
            VkDeviceMemory shadowDepthImageMemory;
            VkImageView shadowDepthImageView;
            VkSampler shadowDepthImageSampler;

            BananDevice &device;
    };
}