//
// Created by yashr on 12/30/21.
//

#pragma once

#include "banan_device.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vk_mem_alloc.h>

namespace Banan {
    class BananModel {
        public:

            struct Vertex {
                glm::vec3 position;
                glm::vec3 color;

                static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
                static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
            };

            struct Builder {
                std::vector<Vertex> vertices{};
                std::vector<uint32_t> indices{};
            };

            BananModel(BananDevice &device, const Builder &builder);
            ~BananModel();

            BananModel(const BananModel &) = delete;
            BananModel &operator=(const BananModel &) = delete;

            void bind(VkCommandBuffer commandBuffer);
            void draw(VkCommandBuffer commandBuffer);

        private:
            void createVertexBuffers(const std::vector<Vertex> &vertices);
            void createIndexBuffers(const std::vector<uint32_t> &indices);

            bool hasIndexBuffer;
            BananDevice &bananDevice;

            VkBuffer vertexBuffer;
            VkDeviceMemory vertexBufferMemory;
            uint32_t vertexCount;

            VkBuffer indexBuffer;
            VkDeviceMemory indexBufferMemory;
            uint32_t indexCount;
    };
}
