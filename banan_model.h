//
// Created by yashr on 12/30/21.
//

#pragma once

#include "banan_device.h"
#include "banan_buffer.h"
#include "banan_image.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace Banan {
    class BananModel {
    public:
        struct Texture {
            void *data = nullptr;
            size_t stride = 0;
            uint32_t width = 0;
            uint32_t height = 0;
            uint32_t mipLevels = 1;
        };

        struct Vertex {
            glm::vec3 color;
            glm::vec3 normal;
            glm::vec3 tangent;
            glm::vec2 uv;

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
            static std::vector<VkVertexInputBindingDescription> getPositionOnlyBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getPositionOnlyAttributeDescriptions();
        };

        struct Builder {
            std::vector<glm::vec3> positions{};
            std::vector<Vertex> misc{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string &filepath);
        };

        BananModel(BananDevice &device, const Builder &builder);
        ~BananModel();

        BananModel(const BananModel &) = delete;
        BananModel &operator=(const BananModel &) = delete;

        static std::shared_ptr<BananModel> createModelFromFile(BananDevice &device, const std::string &filepath);

        void bindPosition(VkCommandBuffer commandBuffer);
        void bindAll(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffers(const std::vector<glm::vec3> &vertices, const std::vector<Vertex> &misc);
        void createIndexBuffers(const std::vector<uint32_t> &indices);

        bool hasIndexBuffer;

        BananDevice &bananDevice;

        std::unique_ptr<BananBuffer> vertexBuffer;
        std::unique_ptr<BananBuffer> miscBuffer;
        uint32_t vertexCount;

        std::unique_ptr<BananBuffer> indexBuffer;
        uint32_t indexCount;
    };
}
