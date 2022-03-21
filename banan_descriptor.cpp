//
// Created by yashr on 2/22/22.
//

#include "banan_descriptor.h"
#include "banan_logger.h"

// std
#include <cassert>
#include <stdexcept>

namespace Banan {

    BananDescriptorSetLayout::Builder &BananDescriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count) {
        assert(bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        bindings[binding] = layoutBinding;
        return *this;
    }

    std::unique_ptr<BananDescriptorSetLayout> BananDescriptorSetLayout::Builder::build() const {
        return std::make_unique<BananDescriptorSetLayout>(bananDevice, bindings);
    }

    BananDescriptorSetLayout::BananDescriptorSetLayout(BananDevice &bananDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) : bananDevice{bananDevice}, bindings{bindings} {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto kv : bindings) {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(bananDevice.device(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    BananDescriptorSetLayout::~BananDescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(bananDevice.device(), descriptorSetLayout, nullptr);
    }

    BananDescriptorPool::Builder &BananDescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count) {
        poolSizes.push_back({descriptorType, count});
        return *this;
    }

    BananDescriptorPool::Builder &BananDescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags) {
        poolFlags = flags;
        return *this;
    }
    BananDescriptorPool::Builder &BananDescriptorPool::Builder::setMaxSets(uint32_t count) {
        maxSets = count;
        return *this;
    }

    std::unique_ptr<BananDescriptorPool> BananDescriptorPool::Builder::build() const {
        return std::make_unique<BananDescriptorPool>(bananDevice, maxSets, poolFlags, poolSizes);
    }


    BananDescriptorPool::BananDescriptorPool(BananDevice &bananDevice, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize> &poolSizes) : bananDevice{bananDevice} {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(bananDevice.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    BananDescriptorPool::~BananDescriptorPool() {
        vkDestroyDescriptorPool(bananDevice.device(), descriptorPool, nullptr);
    }

    bool BananDescriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        if (vkAllocateDescriptorSets(bananDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
            return false;
        }

        return true;
    }

    void BananDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {
        vkFreeDescriptorSets(bananDevice.device(), descriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
    }

    void BananDescriptorPool::resetPool() {
        vkResetDescriptorPool(bananDevice.device(), descriptorPool, 0);
    }

    BananDescriptorWriter::BananDescriptorWriter(BananDescriptorSetLayout &setLayout, BananDescriptorPool &pool) : setLayout{setLayout}, pool{pool} {}

    BananDescriptorWriter &BananDescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
        auto &bindingDescription = setLayout.bindings[binding];
        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    BananDescriptorWriter &BananDescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
        auto &bindingDescription = setLayout.bindings[binding];
        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    bool BananDescriptorWriter::build(VkDescriptorSet &set) {
        bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
        if (!success) {
            return false;
        }

        overwrite(set);
        return true;
    }

    void BananDescriptorWriter::overwrite(VkDescriptorSet &set) {
        for (auto &write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.bananDevice.device(), writes.size(), writes.data(), 0, nullptr);
    }

}