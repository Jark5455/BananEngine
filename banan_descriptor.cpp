//
// Created by yashr on 2/22/22.
//

#include "banan_descriptor.h"

// std
#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <utility>

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

    std::shared_ptr<BananDescriptorSetLayout> BananDescriptorSetLayout::Builder::build() const {
        return std::make_shared<BananDescriptorSetLayout>(bananDevice, bindings, flags);
    }

    BananDescriptorSetLayout::Builder &BananDescriptorSetLayout::Builder::addFlags(uint32_t binding, VkDescriptorBindingFlagsEXT flag) {
        flags.emplace(binding, flag);
        return *this;
    }

    BananDescriptorSetLayout::BananDescriptorSetLayout(BananDevice &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindingsMap, std::unordered_map<uint32_t, VkDescriptorBindingFlagsEXT> flags) : bananDevice{device}, bindings{std::move(bindingsMap)} {
        auto getVal = [](auto const& pair) {return pair.second;};

        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        std::vector<VkDescriptorBindingFlagsEXT> setLayoutFlags{};

        std::transform(bindings.begin(), bindings.end(), std::back_inserter(setLayoutBindings), getVal);
        std::transform(flags.begin(), flags.end(), std::back_inserter(setLayoutFlags), getVal);

        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT descriptorSetLayoutBindingFlagsCreateInfoExt{};
        descriptorSetLayoutBindingFlagsCreateInfoExt.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
        descriptorSetLayoutBindingFlagsCreateInfoExt.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutBindingFlagsCreateInfoExt.pBindingFlags = setLayoutFlags.data();
        descriptorSetLayoutBindingFlagsCreateInfoExt.pNext = nullptr;

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        descriptorSetLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        descriptorSetLayoutInfo.pNext = &descriptorSetLayoutBindingFlagsCreateInfoExt;

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

    std::shared_ptr<BananDescriptorPool> BananDescriptorPool::Builder::build() const {
        return std::make_shared<BananDescriptorPool>(bananDevice, maxSets, poolFlags, poolSizes);
    }

    BananDescriptorPool::BananDescriptorPool(BananDevice &device, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize> &poolSizes) : bananDevice{device} {
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

    bool BananDescriptorPool::allocateDescriptor(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor, std::vector<uint32_t> descriptorCount) {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        VkDescriptorSetVariableDescriptorCountAllocateInfoEXT descriptorSetVariableDescriptorCountAllocateInfoExt{};
        descriptorSetVariableDescriptorCountAllocateInfoExt.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
        descriptorSetVariableDescriptorCountAllocateInfoExt.descriptorSetCount = static_cast<uint32_t>(descriptorCount.size());
        descriptorSetVariableDescriptorCountAllocateInfoExt.pDescriptorCounts = descriptorCount.data();

        allocInfo.pNext = descriptorCount.empty() ? nullptr : &descriptorSetVariableDescriptorCountAllocateInfoExt;

        if (vkAllocateDescriptorSets(bananDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS) {
            return false;
        }

        return true;
    }

    bool BananDescriptorPool::allocateDescriptor(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pNext = nullptr;

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

    BananDescriptorWriter::BananDescriptorWriter(BananDescriptorSetLayout &descriptorSetLayout, BananDescriptorPool &descriptorPool) : setLayout{descriptorSetLayout}, pool{descriptorPool} {}

    BananDescriptorWriter &BananDescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo &bufferInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
        auto &bindingDescription = setLayout.bindings[binding];
        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = &bufferInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    BananDescriptorWriter &BananDescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo &imageInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
        auto &bindingDescription = setLayout.bindings[binding];
        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = &imageInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    BananDescriptorWriter &BananDescriptorWriter::writeImages(uint32_t binding, const std::unordered_map<uint32_t, VkDescriptorImageInfo>& imageInfos) {
        auto &bindingDescription = setLayout.bindings[binding];
        for (auto &kv : imageInfos) {
            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorType = bindingDescription.descriptorType;
            write.dstBinding = binding;
            write.dstArrayElement = kv.first;
            write.pImageInfo = &kv.second;
            write.descriptorCount = 1;

            writes.push_back(write);
        }

        return *this;
    }

    BananDescriptorWriter &BananDescriptorWriter::writeImages(uint32_t binding, const std::vector<VkDescriptorImageInfo>& imageInfos) {
        auto &bindingDescription = setLayout.bindings[binding];

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.dstArrayElement = 0;
        write.pImageInfo = imageInfos.data();
        write.descriptorCount = static_cast<uint32_t>(imageInfos.size());

        writes.push_back(write);

        return *this;
    }

    bool BananDescriptorWriter::build(VkDescriptorSet &set, std::vector<uint32_t> descriptorCount) {
        bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set, std::move(descriptorCount));
        if (!success) {
            return false;
        }
        overwrite(set);
        return true;
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

        vkUpdateDescriptorSets(pool.bananDevice.device(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }
}
