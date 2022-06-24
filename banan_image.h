//
// Created by yashr on 3/13/22.
//

#pragma once

#include "banan_device.h"

#define STB_IMAGE_IMPLEMENTATION

namespace Banan {
    class BananImage {
    public:
        BananImage(BananDevice &device, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkSampleCountFlagBits numSamples, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment = 1);
        ~BananImage();

        BananImage(const BananImage&) = delete;
        BananImage& operator=(const BananImage&) = delete;

        void generateMipMaps(VkFormat imageFormat);

        VkDescriptorImageInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        VkImage getImageHandle();
        VkImageView getImageViewHandle();
        VkSampler getImageSamplerHandle();

    private:

        void createTextureImageView();
        void createTextureSampler();

        static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

        BananDevice &bananDevice;
        VkImage image;
        VkFormat imageFormat;
        VkImageView imageView;
        VkSampler imageSampler;
        VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkDeviceMemory memory;

        uint32_t width;
        uint32_t height;
        uint32_t mipLevels;
    };
}
