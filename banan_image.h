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

            BananImage(BananDevice &device, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkSampleCountFlagBits numSamples, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
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
            void createTextureSampler();

            BananDevice &bananDevice;
            VkImage image;
            VkFormat imageFormat;
            VkImageView imageView;
            VkSampler imageSampler;
            VkDeviceMemory memory;
            uint32_t mipLevels;
            VkExtent2D imageExtent;
            VkImageLayout imageLayout;
    };

    class BananImageArray : public BananImage {
        public:
            BananImageArray(BananDevice &device, uint32_t width, uint32_t height, uint32_t arrayLevels, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkSampleCountFlagBits numSamples, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
        private:
            void createTextureImageView();
            uint32_t arrayLevels;
            uint32_t mipLevels;
    };

    class BananCubemap {
        public:
            BananCubemap(BananDevice &device, uint32_t sideLength, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkSampleCountFlagBits numSamples, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags);
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
            uint32_t mipLevels;
    };
}
