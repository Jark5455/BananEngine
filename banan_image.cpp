//
// Created by yashr on 3/13/22.
//

#include "banan_image.h"

#include <stdexcept>

namespace Banan {
    VkDeviceSize BananImage::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
        if (minOffsetAlignment > 0) {
            return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
        }
        return instanceSize;
    }

    BananImage::BananImage(BananDevice &device, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkSampleCountFlagBits numSamples, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment) : bananDevice{device}, width{width}, height{height}, mipLevels{mipLevels}, imageFormat{format} {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(width);
        imageInfo.extent.height = static_cast<uint32_t>(height);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = imageFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usageFlags;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = numSamples;

        bananDevice.createImageWithInfo(imageInfo, memoryPropertyFlags, image, memory);
        imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        createTextureImageView();
        createTextureSampler();
    }

    BananImage::~BananImage() {
        vkDestroySampler(bananDevice.device(), imageSampler, nullptr);
        vkDestroyImageView(bananDevice.device(), imageView, nullptr);
        vkDestroyImage(bananDevice.device(), image, nullptr);
        vkFreeMemory(bananDevice.device(), memory, nullptr);
    }

    VkDescriptorImageInfo BananImage::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        VkDescriptorImageInfo info{};
        info.imageView = imageView;
        info.imageLayout = imageLayout;
        info.sampler = imageSampler;
        return info;
    }

    void BananImage::createTextureImageView() {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = imageFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(bananDevice.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    void BananImage::createTextureSampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = bananDevice.physicalDeviceProperties().limits.maxSamplerAnisotropy;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(mipLevels);

        if (vkCreateSampler(bananDevice.device(), &samplerInfo, nullptr, &imageSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    VkImage BananImage::getImageHandle() {
        return image;
    }

    VkImageView BananImage::getImageViewHandle() {
        return imageView;
    }

    VkSampler BananImage::getImageSamplerHandle() {
        return imageSampler;
    }
}
