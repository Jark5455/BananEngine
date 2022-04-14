//
// Created by yashr on 3/13/22.
//

#pragma once

#include "banan_device.h"

namespace Banan {
    class BananImage {
        public:
            BananImage(BananDevice& device, VkDeviceSize pixelSize, uint32_t width, uint32_t height, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment = 1);
            ~BananImage();

            BananImage(const BananImage&) = delete;
            BananImage& operator=(const BananImage&) = delete;

            VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
            void unmap();

            void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
            VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
            VkDescriptorImageInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
            VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

            void writeToIndex(void* data, int index);
            VkResult flushIndex(int index);
            VkDescriptorImageInfo descriptorInfoForIndex(int index);
            VkResult invalidateIndex(int index);

            VkImage getImage();
            void* getMappedMemory();
            uint32_t getWidth() const;
            uint32_t getHeight() const;
            VkDeviceSize getPixelSize() const;
            VkDeviceSize getAlignmentSize() const;
            VkBufferUsageFlags getUsageFlags() const;
            VkMemoryPropertyFlags getMemoryPropertyFlags() const;
            VkDeviceSize getBufferSize() const;

        private:
            static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

            BananDevice& bananDevice;
            void* mapped = nullptr;
            VkImage image = VK_NULL_HANDLE;
            VkDeviceMemory memory = VK_NULL_HANDLE;

            VkDeviceSize imageSize;
            uint32_t width;
            uint32_t height;
            VkDeviceSize pixelSize;
            VkDeviceSize alignmentSize;
            VkBufferUsageFlags usageFlags;
            VkMemoryPropertyFlags memoryPropertyFlags;
    };
}
