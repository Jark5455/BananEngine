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

    BananImage::BananImage(BananDevice &device, VkDeviceSize pixelSize, uint32_t width, uint32_t height, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment) : bananDevice{device}, width{width}, height{height} {
        bananDevice.createImage(width, height, usageFlags, memoryPropertyFlags, image, memory);
    }

    BananImage::~BananImage() {
        vkDestroyImage(bananDevice.device(), image, nullptr);
        vkFreeMemory(bananDevice.device(), memory, nullptr);
    }

    void BananImage::createTextureImageView() {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(bananDevice.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    VkDescriptorImageInfo BananImage::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        VkDescriptorImageInfo info{};
        info.imageView = imageView;
        info.imageLayout = imageLayout;

        return info;
    }

    VkImage BananImage::getImage() {
        return image;
    }
}