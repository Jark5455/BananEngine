//
// Created by yashr on 12/30/21.
//

#include "banan_model.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <iterator>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Banan {
    BananModel::BananModel(BananDevice &device, const Builder &builder) : bananDevice(device) {
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }

    BananModel::~BananModel() {
        vkDestroyBuffer(bananDevice.device(), vertexBuffer, nullptr);
        vkFreeMemory(bananDevice.device(), vertexBufferMemory, nullptr);
    }

    void BananModel::bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        }
    }

    void BananModel::draw(VkCommandBuffer commandBuffer) {

        if (hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        } else {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }

    void BananModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be atleast 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        bananDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void *data;
        vkMapMemory(bananDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(bananDevice.device(), stagingBufferMemory);

        bananDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);
        bananDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(bananDevice.device(), stagingBuffer, nullptr);
        vkFreeMemory(bananDevice.device(), stagingBufferMemory, nullptr);
    }

    void BananModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;

        if (!hasIndexBuffer) {
            return;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        bananDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void *data;
        vkMapMemory(bananDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(bananDevice.device(), stagingBufferMemory);

        bananDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBuffer, indexBufferMemory);
        bananDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

        vkDestroyBuffer(bananDevice.device(), stagingBuffer, nullptr);
        vkFreeMemory(bananDevice.device(), stagingBufferMemory, nullptr);
    }

    std::unique_ptr<BananModel> BananModel::createModelFromFile(BananDevice &device, const std::string &filepath) {
        Builder builder{};
        builder.loadModel(filepath);
        std::cout << "Model Vertex Count:" << builder.vertices.size() << '\n';
        return std::make_unique<BananModel>(device, builder);
    }

    std::vector<VkVertexInputBindingDescription> BananModel::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> BananModel::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }

    void BananModel::Builder::loadModel(const std::string &filepath) {
        Assimp::Importer importer;

        const aiScene *scene = importer.ReadFile(filepath,aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

        vertices.clear();
        indices.clear();

        if (nullptr == scene) {
            throw std::runtime_error(importer.GetErrorString());
        }

        for (size_t i = 0; i < scene->mNumMeshes; ++i) {
            for (size_t j = 0; i < scene->mMeshes[i]->mNumVertices; ++j) {

                const auto *mesh = &*scene->mMeshes[i];

                const auto pPos = mesh.HasPositions() ? mesh.mVertices[j] : aiVector3t<float>(0.f, 0.f, 0.f);
                const auto pNormal = mesh.HasNormals() ? mesh.mNormals[j] : aiVector3t<float>(0.f, 0.f, 0.f);
                const auto pTexCoord = mesh.HasTextureCoords(i) ? mesh.mTextureCoords[0][j] : aiVector3t<float>(0.f, 0.f, 0.f);

                Vertex vertex{};
                //vertex.position = scene->mMeshes[i]->HasPositions() ? glm::vec3{scene->mMeshes[i]->mVertices[j].x, scene->mMeshes[i]->mVertices[j].y, scene->mMeshes[i]->mVertices[j].z} : glm::vec3{0.0f, 0.0f, 0.0f};
                //vertex.normal = scene->mMeshes[i]->HasNormals() ? glm::vec3{scene->mMeshes[i]->mNormals[j].x, scene->mMeshes[i]->mNormals[j].y, scene->mMeshes[i]->mNormals[j].z} : glm::vec3{0.0f, 0.0f, 0.0f};
                //vertex.color = scene->mMeshes[i]->HasVertexColors(j) ? glm::vec3{scene->mMeshes[i]->mColors[j]->r, scene->mMeshes[i]->mColors[j]->g, scene->mMeshes[i]->mColors[j]->b} : glm::vec3{0.0f, 0.0f, 0.0f};
                //vertex.uv = scene->mMeshes[i]->HasTextureCoordsName(j) ? glm::vec3{scene->mMeshes[i]->mTextureCoords[j]->x, scene->mMeshes[i]->mTextureCoords[j]->y, NULL} : glm::vec3{0.0f, 0.0f, 0.0f};

                vertex.position = {pPos.x, pPos.y, pPos.z};
                vertex.normal = {pNormal.x, pNormal.y, pNormal.z};
                vertex.uv = {pTexCoord.x, pTexCoord.y, NULL};

                vertices.push_back(vertex);
            }
        }
    }
}
