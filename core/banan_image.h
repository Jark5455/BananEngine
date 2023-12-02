//
// Created by yashr on 3/13/22.
//

#pragma once

#include "banan_device.h"

#define STB_IMAGE_IMPLEMENTATION

namespace Banan {
    class BananImage {
    public:
        BananImage(BananDevice &device, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkSampleCountFlagBits numSamples, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
        ~BananImage();

        BananImage(const BananImage&) = delete;
        BananImage& operator=(const BananImage&) = delete;

        VkDescriptorImageInfo descriptorInfo();
        VkImage getImageHandle();

    private:

        void createTextureImageView();
        void createTextureSampler();

        BananDevice &bananDevice;
        VkImage image;
        VkFormat imageFormat;
        VkImageView imageView;
        VkSampler imageSampler;
        VkDeviceMemory memory;
        uint32_t mipLevels;
    };

    class BananCubemap {
    public:
        BananCubemap(BananDevice &device, uint32_t sideLength, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkSampleCountFlagBits numSamples, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment = 1);
        ~BananCubemap();

        BananCubemap(const BananCubemap&) = delete;
        BananCubemap& operator=(const BananCubemap&) = delete;

        VkDescriptorImageInfo descriptorInfo();
        VkImage getImageHandle();

    private:
        void createTextureImageView();
        void createTextureSampler();

        BananDevice &bananDevice;
        VkFormat cubemapImageFormat;
        VkImage cubemapImage;
        VkImageView cubemapImageView;
        VkSampler cubemapImageSampler;
        VkDeviceMemory memory;
        uint32_t mipLevels;
    };
}