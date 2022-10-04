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
            uint32_t mipLevels;
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
            Texture texture{};

            void loadModel(const std::string &filepath);
            void loadTexture(const std::string &filepath);
        };

        BananModel(BananDevice &device, const Builder &builder);
        ~BananModel();

        BananModel(const BananModel &) = delete;
        BananModel &operator=(const BananModel &) = delete;

        static std::unique_ptr<BananModel> createModelFromFile(BananDevice &device, const std::string &filepath);

        void bindPosition(VkCommandBuffer commandBuffer);
        void bindAll(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

        bool isTextureLoaded();
        VkDescriptorImageInfo getDescriptorImageInfo();

    private:
        void createVertexBuffers(const std::vector<glm::vec3> &vertices, const std::vector<Vertex> &misc);
        void createIndexBuffers(const std::vector<uint32_t> &indices);
        void createTextureImage(const Texture &image);

        bool hasIndexBuffer;
        bool hasTexture;

        BananDevice &bananDevice;

        std::unique_ptr<BananBuffer> vertexBuffer;
        std::unique_ptr<BananBuffer> miscBuffer;
        uint32_t vertexCount;

        std::unique_ptr<BananBuffer> indexBuffer;
        uint32_t indexCount;

        std::unique_ptr<BananImage> textureImage;
        uint32_t pixelCount;
    };
}