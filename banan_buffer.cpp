//
// Created by yashr on 2/20/22.
//

#include "banan_buffer.h"

// std
#include <cassert>
#include <cstring>

namespace Banan {

    VkDeviceSize BananBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment) {
        if (minOffsetAlignment > 0) {
            return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
        }
        return instanceSize;
    }

    BananBuffer::BananBuffer(BananDevice &device, VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize minOffsetAlignment) : bananDevice{device}, instanceCount{instanceCount}, instanceSize{instanceSize}, usageFlags{usageFlags}, memoryPropertyFlags{memoryPropertyFlags} {
        alignmentSize = getAlignment(instanceSize, minOffsetAlignment);
        bufferSize = alignmentSize * instanceCount;
        device.createBuffer(bufferSize, usageFlags, memoryPropertyFlags, buffer, memory);
    }

    BananBuffer::~BananBuffer() {
        unmap();
        vkDestroyBuffer(bananDevice.device(), buffer, nullptr);
        vkFreeMemory(bananDevice.device(), memory, nullptr);
    }

    VkResult BananBuffer::map(VkDeviceSize size, VkDeviceSize offset) {
        assert(buffer && memory && "Called map on buffer before create");
        return vkMapMemory(bananDevice.device(), memory, offset, size, 0, &mapped);
    }

    void BananBuffer::unmap() {
        if (mapped) {
            vkUnmapMemory(bananDevice.device(), memory);
            mapped = nullptr;
        }
    }

    void BananBuffer::writeToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset) {
        assert(mapped && "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE) {
            memcpy(mapped, data, bufferSize);
        } else {
            char *memOffset = (char *)mapped;
            memOffset += offset;
            memcpy(memOffset, data, size);
        }
    }

    VkResult BananBuffer::flush(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(bananDevice.device(), 1, &mappedRange);
    }

    VkResult BananBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkInvalidateMappedMemoryRanges(bananDevice.device(), 1, &mappedRange);
    }

    VkDescriptorBufferInfo BananBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        return VkDescriptorBufferInfo{buffer, offset,size};
    }

    void BananBuffer::writeToIndex(void *data, int index) {
        writeToBuffer(data, instanceSize, index * alignmentSize);
    }

    VkResult BananBuffer::flushIndex(int index) {
        return flush(alignmentSize, index * alignmentSize);
    }

    VkDescriptorBufferInfo BananBuffer::descriptorInfoForIndex(int index) {
        return descriptorInfo(alignmentSize, index * alignmentSize);
    }

    VkResult BananBuffer::invalidateIndex(int index) {
        return invalidate(alignmentSize, index * alignmentSize);
    }

    VkBuffer BananBuffer::getBuffer() {
        return buffer;
    }

    void *BananBuffer::getMappedMemory() {
        return mapped;
    }

    uint32_t BananBuffer::getInstanceCount() const {
        return instanceCount;
    }

    VkDeviceSize BananBuffer::getInstanceSize() const {
        return instanceSize;
    }

    VkDeviceSize BananBuffer::getAlignmentSize() const {
        return alignmentSize;
    }

    VkBufferUsageFlags BananBuffer::getUsageFlags() const {
        return usageFlags;
    }

    VkMemoryPropertyFlags BananBuffer::getMemoryPropertyFlags() const {
        return memoryPropertyFlags;
    }

    VkDeviceSize BananBuffer::getBufferSize() const {
        return bufferSize;
    }
}