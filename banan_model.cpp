//
// Created by yashr on 12/30/21.
//

#include "banan_model.h"

#include <cassert>

namespace Banan {

    BananModel::BananModel(BananDevice &device, const std::vector<Vertex> &vertices) : bananDevice(device) {
        createVertexBuffers(vertices);
    }

    BananModel::~BananModel() {
        vkDestroyBuffer(bananDevice.device(), vertexBuffer, nullptr);
        vkFreeMemory(bananDevice.device(), vertexBufferMemory, nullptr);
    }

    void BananModel::bind(VkCommandBuffer commandBuffer) {

    }

    void BananModel::draw(VkCommandBuffer commandBuffer) {

    }

    void BananModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be atleast 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
    }
}
