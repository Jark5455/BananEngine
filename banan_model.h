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
                uint8_t *data = nullptr;
                uint32_t width;
                uint32_t height;
            };

            struct Vertex {
                glm::vec3 position;
                glm::vec3 color;
                glm::vec3 normal;
                glm::vec2 uv;

                static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
                static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
            };

            struct Builder {
                std::vector<Vertex> vertices{};
                std::vector<uint32_t> indices{};
                Texture texture{};

                void loadModel(const std::string &filepath);
                void loadTexture(const std::string &filepath);
            };

            BananModel(BananDevice &device, const Builder &builder);
            ~BananModel();

            BananModel(const BananModel &) = delete;
            BananModel &operator=(const BananModel &) = delete;

            static std::unique_ptr<BananModel> createModelFromFile(BananDevice &device, const std::string &filepath);

            void bind(VkCommandBuffer commandBuffer);
            void draw(VkCommandBuffer commandBuffer);

            bool isTextureLoaded();
            VkDescriptorImageInfo getDescriptorImageInfo();

        private:
            void createVertexBuffers(const std::vector<Vertex> &vertices);
            void createIndexBuffers(const std::vector<uint32_t> &indices);
            void createTextureImage(const Texture &image);

            bool hasIndexBuffer;
            bool hasTexture;

            BananDevice &bananDevice;

            std::unique_ptr<BananBuffer> vertexBuffer;
            uint32_t vertexCount;

            std::unique_ptr<BananBuffer> indexBuffer;
            uint32_t indexCount;

            std::unique_ptr<BananImage> textureImage;
            uint32_t pixelCount;
    };
}
