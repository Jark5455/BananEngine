//
// Created by yashr on 3/13/22.
//

#pragma once

#include "banan_device.h"

#include <memory>

#define STB_IMAGE_IMPLEMENTATION

namespace Banan {
    class BananImage {
        public:

            static std::shared_ptr<BananImage> makeImageFromFilepath(BananDevice &device, const std::string &filepath);

            BananImage(BananDevice &device, size_t width, size_t height, size_t mipLevels, size_t arrayLevels, VkFormat format, VkImageTiling tiling, VkSampleCountFlagBits numSamples, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
            ~BananImage();

            BananImage(const BananImage&) = delete;
            BananImage& operator=(const BananImage&) = delete;

            VkDescriptorImageInfo descriptorInfo();
            VkImage getImageHandle();
            VkImageView getImageView();
            VkSampler getImageSampler();
            VkExtent2D getImageExtent();
            VkFormat getImageFormat();
            VkImageLayout getImageLayout();

            void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);
            void generateMipMaps(uint32_t mipLevels);

        protected:
            BananImage(BananDevice &device, VkFormat format);

            void createTextureImageView();
            void createTextureArrayImageView();
            void createTextureSampler();

            BananDevice &bananDevice;
            VkImage image;
            VkFormat imageFormat;
            VkImageView imageView;
            VkSampler imageSampler;
            VkDeviceMemory memory;
            size_t mipLevels;
            size_t arrayLevels;
            VkExtent2D imageExtent;
            VkImageLayout imageLayout;
    };

    class BananCubemap {
        public:
            BananCubemap(BananDevice &device, size_t sideLength, size_t mipLevels, VkFormat format, VkImageTiling tiling, VkSampleCountFlagBits numSamples, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
            ~BananCubemap();

            BananCubemap(const BananCubemap&) = delete;
            BananCubemap& operator=(const BananCubemap&) = delete;

            VkDescriptorImageInfo cubemapDescriptorInfo();

            VkImage getImageHandle();

        private:
            void createTextureCubemapImageView();
            void createTextureSampler();

            BananDevice &bananDevice;
            VkFormat cubemapImageFormat;
            VkImage cubemapImage;
            VkImageView cubemapImageView;
            VkSampler cubemapImageSampler;
            VkDeviceMemory memory;
            size_t mipLevels;
    };
}
