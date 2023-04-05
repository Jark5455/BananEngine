//
// Created by yashr on 2/22/22.
//

#pragma once

#include "banan_device.h"

#include <array>
#include <memory>
#include <unordered_map>

namespace Banan {
    class BananDescriptorSetLayout {
        public:
            class Builder {
                public:
                    Builder(BananDevice &bananDevice) : bananDevice{bananDevice} {}
                    Builder &addFlag(VkDescriptorBindingFlagsEXT flag);
                    Builder &addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1);
                    std::unique_ptr<BananDescriptorSetLayout> build() const;

                private:
                    BananDevice &bananDevice;
                    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
                    std::vector<VkDescriptorBindingFlagsEXT> flags{};
            };

            BananDescriptorSetLayout(BananDevice &bananDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings, std::vector<VkDescriptorBindingFlagsEXT> flags);
            ~BananDescriptorSetLayout();
            BananDescriptorSetLayout(const BananDescriptorSetLayout &) = delete;
            BananDescriptorSetLayout &operator=(const BananDescriptorSetLayout &) = delete;

            VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

        private:
            BananDevice &bananDevice;
            VkDescriptorSetLayout descriptorSetLayout;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

            friend class BananDescriptorWriter;
    };

    class BananDescriptorPool {
        public:
            class Builder {
                public:
                    Builder(BananDevice &bananDevice) : bananDevice{bananDevice} {}

                    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
                    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
                    Builder &setMaxSets(uint32_t count);
                    std::unique_ptr<BananDescriptorPool> build() const;

                private:
                    BananDevice &bananDevice;
                    std::vector<VkDescriptorPoolSize> poolSizes{};
                    uint32_t maxSets = 1000;
                    VkDescriptorPoolCreateFlags poolFlags = 0;
            };

            BananDescriptorPool(BananDevice &bananDevice, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize> &poolSizes);
            ~BananDescriptorPool();
            BananDescriptorPool(const BananDescriptorPool &) = delete;
            BananDescriptorPool &operator=(const BananDescriptorPool &) = delete;

            bool allocateDescriptor(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor, std::vector<uint32_t> descriptorCount);
            bool allocateDescriptor(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor);

            void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;
            void resetPool();

        private:
            BananDevice &bananDevice;
            VkDescriptorPool descriptorPool;
            friend class BananDescriptorWriter;
    };

    class BananDescriptorWriter {
        public:
            BananDescriptorWriter(BananDescriptorSetLayout &setLayout, BananDescriptorPool &pool);

            BananDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo &bufferInfo);
            BananDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo &imageInfo);
            BananDescriptorWriter &writeImages(uint32_t binding, const std::unordered_map<uint32_t, VkDescriptorImageInfo>& imageInfos);
            BananDescriptorWriter &writeImages(uint32_t binding, const std::vector<VkDescriptorImageInfo>& imageInfos);

            bool build(VkDescriptorSet &set, std::vector<uint32_t> descriptorCount);
            bool build(VkDescriptorSet &set);

            void overwrite(VkDescriptorSet &set);

        private:
            BananDescriptorSetLayout &setLayout;
            BananDescriptorPool &pool;
            std::vector<VkWriteDescriptorSet> writes;
    };
}