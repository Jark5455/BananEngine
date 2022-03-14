//
// Created by yashr on 3/13/22.
//

#include "banan_image.h"

#include <cassert>
#include <cstring>

namespace Banan {
    VkDeviceSize BananImage::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
        if (minOffsetAlignment > 0) {
            return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
        }
        return instanceSize;
    }

    BananImage::BananImage(BananDevice &device, VkDeviceSize pixelSize, uint32_t width, uint32_t height, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment) : bananDevice{device}, pixelSize{pixelSize}, width{width}, height{height} {
        alignmentSize = getAlignment(pixelSize, minOffsetAlignment);
        imageSize = alignmentSize * width * height;
        device.createImage(width, height, usageFlags, memoryPropertyFlags, image, memory);
    }

    BananImage::~BananImage() {

    }

    VkResult BananImage::map(VkDeviceSize size, VkDeviceSize offset) {
        assert(image && memory && "Called map on buffer before create");
        return vkMapMemory(bananDevice.device(), memory, offset, size, 0, &mapped);
    }

    void BananImage::unmap() {
        if (mapped) {
            vkUnmapMemory(bananDevice.device(), memory);
            mapped = nullptr;
        }
    }

    void BananImage::writeToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset) {
        assert(mapped && "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE) {
            memcpy(mapped, data, imageSize);
        } else {
            char *memOffset = (char *)mapped;
            memOffset += offset;
            memcpy(memOffset, data, size);
        }
    }

    VkResult BananImage::flush(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(bananDevice.device(), 1, &mappedRange);
    }

    //TODO
    //VkDescriptorImageInfo BananImage::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
    //    return VkDescriptorImageInfo{image, offset, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    //}

    VkResult BananImage::invalidate(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkInvalidateMappedMemoryRanges(bananDevice.device(), 1, &mappedRange);
    }

    void BananImage::writeToIndex(void *data, int index) {
        writeToBuffer(data, pixelSize, index * alignmentSize);
    }

    VkResult BananImage::flushIndex(int index) {
        return flush(alignmentSize, index * alignmentSize);
    }

    //TODO
    //VkDescriptorImageInfo BananImage::descriptorInfoForIndex(int index) {
    //    return descriptorInfo(alignmentSize, index * alignmentSize);
    //}

    VkResult BananImage::invalidateIndex(int index) {
        return invalidate(alignmentSize, index * alignmentSize);
    }

    VkImage BananImage::getImage() {
        return image;
    }

    void *BananImage::getMappedMemory() {
        return mapped;
    }

    uint32_t BananImage::getWidth() const {
        return width;
    }

    uint32_t BananImage::getHeight() const {
        return height;
    }

    VkDeviceSize BananImage::getPixelSize() const {
        return pixelSize;
    }

    VkDeviceSize BananImage::getAlignmentSize() const {
        return alignmentSize;
    }

    VkBufferUsageFlags BananImage::getUsageFlags() const {
        return usageFlags;
    }

    VkMemoryPropertyFlags BananImage::getMemoryPropertyFlags() const {
        return memoryPropertyFlags;
    }

    VkDeviceSize BananImage::getBufferSize() const {
        return imageSize;
    }
}