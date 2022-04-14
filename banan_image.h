//
// Created by yashr on 3/13/22.
//

#pragma once

#include "banan_device.h"

#define STB_IMAGE_IMPLEMENTATION

namespace Banan {
    class BananImage {
        public:
            BananImage(BananDevice& device, VkDeviceSize pixelSize, uint32_t width, uint32_t height, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment = 1);
            ~BananImage();

            BananImage(const BananImage&) = delete;
            BananImage& operator=(const BananImage&) = delete;

            void createTextureImageView();
            VkDescriptorImageInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

            VkImage getImage();

        private:
            static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

            BananDevice& bananDevice;
            VkImage image = VK_NULL_HANDLE;
            VkImageView imageView = VK_NULL_HANDLE;
            VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkDeviceMemory memory = VK_NULL_HANDLE;

            uint32_t width;
            uint32_t height;
    };
}
