//
// Created by yashr on 12/30/21.
//

#pragma once

#include "banan_device.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Banan {
    class BananModel {
        public:

            struct Vertex {
                glm::vec2 position;
                static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
                static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
            };

            BananModel(BananDevice &device, const std::vector<Vertex> &vertices);
            ~BananModel();

            BananModel(const BananModel &) = delete;
            BananModel &operator=(const BananModel &) = delete;

            void bind(VkCommandBuffer commandBuffer);
            void draw(VkCommandBuffer commandBuffer);

        private:
            void createVertexBuffers(const std::vector<Vertex> &vertices);

            BananDevice &bananDevice;
            VkBuffer vertexBuffer;
            VkDeviceMemory vertexBufferMemory;
            uint32_t vertexCount;
    };
}
