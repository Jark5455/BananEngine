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
                    Builder(BananDevice &device) : bananDevice{device} {}
                    Builder &addFlag(VkDescriptorBindingFlagsEXT flag);
                    Builder &addBinding(size_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, size_t count = 1);
                    std::unique_ptr<BananDescriptorSetLayout> build() const;

                private:
                    BananDevice &bananDevice;
                    std::unordered_map<size_t, VkDescriptorSetLayoutBinding> bindings{};
                    std::vector<VkDescriptorBindingFlagsEXT> flags{};
            };

            BananDescriptorSetLayout(BananDevice &bananDevice, std::unordered_map<size_t, VkDescriptorSetLayoutBinding> bindings, std::vector<VkDescriptorBindingFlagsEXT> flags);
            ~BananDescriptorSetLayout();
            BananDescriptorSetLayout(const BananDescriptorSetLayout &) = delete;
            BananDescriptorSetLayout &operator=(const BananDescriptorSetLayout &) = delete;

            void resizeBinding(size_t binding, size_t newBindingCount);
            VkDescriptorSetLayout getDescriptorSetLayout() const;

        private:
            BananDevice &bananDevice;
            VkDescriptorSetLayout descriptorSetLayout;
            std::unordered_map<size_t, VkDescriptorSetLayoutBinding> bindings;

            friend class BananDescriptorWriter;
    };

    class BananDescriptorPool {
        public:
            class Builder {
                public:
                    Builder(BananDevice &device) : bananDevice{device} {}

                    Builder &addPoolSize(VkDescriptorType descriptorType, size_t count);
                    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
                    Builder &setMaxSets(size_t count);
                    std::unique_ptr<BananDescriptorPool> build() const;

                private:
                    BananDevice &bananDevice;
                    std::vector<VkDescriptorPoolSize> poolSizes{};
                    size_t maxSets = 1000;
                    VkDescriptorPoolCreateFlags poolFlags = 0;
            };

            BananDescriptorPool(BananDevice &bananDevice, size_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize> &poolSizes);
            ~BananDescriptorPool();
            BananDescriptorPool(const BananDescriptorPool &) = delete;
            BananDescriptorPool &operator=(const BananDescriptorPool &) = delete;

            bool allocateDescriptor(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor, std::vector<size_t> descriptorCount);
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

            BananDescriptorWriter &writeBuffer(size_t binding, VkDescriptorBufferInfo &bufferInfo);
            BananDescriptorWriter &writeImage(size_t binding, VkDescriptorImageInfo &imageInfo);
            BananDescriptorWriter &writeImages(size_t binding, const std::unordered_map<size_t, VkDescriptorImageInfo>& imageInfos);
            BananDescriptorWriter &writeImages(size_t binding, const std::vector<VkDescriptorImageInfo>& imageInfos);

            bool build(VkDescriptorSet &set, std::vector<size_t> descriptorCount);
            bool build(VkDescriptorSet &set);

            void overwrite(VkDescriptorSet &set);

        private:
            BananDescriptorSetLayout &setLayout;
            BananDescriptorPool &pool;
            std::vector<VkWriteDescriptorSet> writes;
    };
}
