//
// Created by yashr on 12/30/21.
//

#include "banan_model.h"
#include "banan_logger.h"

#include <cassert>
#include <cstring>
#include <iterator>
#include <thread>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Banan {
    BananModel::BananModel(BananDevice &device, const Builder &builder) : bananDevice{device} {
        createVertexBuffers(builder.positions, builder.misc);
        createIndexBuffers(builder.indices);
    }

    BananModel::~BananModel() {
    }

    void BananModel::bindPosition(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = {vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    void BananModel::bindAll(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = {vertexBuffer->getBuffer(), miscBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0, 0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 2, buffers, offsets);

        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    void BananModel::draw(VkCommandBuffer commandBuffer) {

        if (hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        } else {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }

    void BananModel::createVertexBuffers(const std::vector<glm::vec3> &vertices, const std::vector<Vertex> &misc) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be atleast 3");
        assert(vertexCount == misc.size() && "Vertex count must be same as misc count");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        BananBuffer stagingBuffer{bananDevice, vertexSize, vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)vertices.data());

        vertexBuffer = std::make_unique<BananBuffer>(bananDevice, vertexSize, vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        bananDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);

        bufferSize = sizeof(misc[0]) * vertexCount;
        uint32_t miscSize = sizeof(misc[0]);

        BananBuffer miscStagingBuffer{bananDevice, miscSize, vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        miscStagingBuffer.map();
        miscStagingBuffer.writeToBuffer((void *)misc.data());

        miscBuffer = std::make_unique<BananBuffer>(bananDevice, miscSize, vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        bananDevice.copyBuffer(miscStagingBuffer.getBuffer(), miscBuffer->getBuffer(), bufferSize);
    }

    void BananModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;

        if (!hasIndexBuffer) {
            return;
        }

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        uint32_t indexSize = sizeof(indices[0]);

        BananBuffer stagingBuffer{bananDevice, indexSize, indexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)indices.data());

        indexBuffer = std::make_unique<BananBuffer>(bananDevice, indexSize, indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        bananDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }

    std::shared_ptr<BananModel> BananModel::createModelFromFile(BananDevice &device, const std::string &filepath) {
        Builder builder{};
        builder.loadModel(filepath);
        std::cout << "Model Vertex Count: " + std::to_string(builder.misc.size());
        return std::make_shared<BananModel>(device, builder);
    }

    std::vector<VkVertexInputBindingDescription> BananModel::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(2);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(glm::vec3);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        bindingDescriptions[1].binding = 1;
        bindingDescriptions[1].stride = sizeof(Vertex);
        bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> BananModel::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0});
        attributeDescriptions.push_back({1, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
        attributeDescriptions.push_back({2, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
        attributeDescriptions.push_back({3, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent)});
        attributeDescriptions.push_back({4, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

        return attributeDescriptions;
    }

    std::vector<VkVertexInputBindingDescription> BananModel::Vertex::getPositionOnlyBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(glm::vec3);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> BananModel::Vertex::getPositionOnlyAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0});
        return attributeDescriptions;
    }

    void BananModel::Builder::loadModel(const std::string &filepath) {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(filepath,
                                                 aiProcess_Triangulate |
                                                 aiProcess_GenSmoothNormals |
                                                 aiProcess_FlipUVs |
                                                 aiProcess_JoinIdenticalVertices |
                                                 aiProcess_GenUVCoords |
                                                 aiProcess_CalcTangentSpace |
                                                 aiProcess_MakeLeftHanded);

        positions.clear();
        misc.clear();
        indices.clear();

        if (scene) {
            for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
                const aiMesh *mesh = scene->mMeshes[i];

                positions.reserve(positions.size() + mesh->mNumVertices);
                misc.reserve(positions.size() + mesh->mNumVertices);
                indices.reserve(positions.size() + mesh->mNumFaces * 3);

                for (uint32_t j = 0; j < mesh->mNumVertices; j++) {
                    Vertex v{};

                    v.color =  mesh->HasVertexColors(j) ? glm::vec3{mesh->mColors[j]->r, mesh->mColors[j]->g, mesh->mColors[j]->b} : glm::vec3{1.0f, 1.0f, 1.0f};
                    v.normal = glm::vec3{mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z};
                    v.tangent = glm::vec3{mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z};
                    v.uv = glm::vec2{mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y};

                    misc.push_back(v);
                    positions.emplace_back( mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);
                }

                for (uint32_t k = 0; k < mesh->mNumFaces; k++) {
                    indices.push_back(mesh->mFaces[k].mIndices[0]);
                    indices.push_back(mesh->mFaces[k].mIndices[1]);
                    indices.push_back(mesh->mFaces[k].mIndices[2]);
                }
            }

            float max_uv_x = 0.0;
            float min_uv_x = 0.0;
            float max_uv_y = 0.0;
            float min_uv_y = 0.0;

            for (Vertex v : misc) {
                if (v.uv.x > max_uv_x) {
                    max_uv_x = v.uv.x;
                }

                if (v.uv.y > max_uv_y) {
                    max_uv_y = v.uv.y;
                }

                if (v.uv.x < min_uv_x) {
                    min_uv_x = v.uv.x;
                }

                if (v.uv.y < min_uv_y) {
                    min_uv_y = v.uv.y;
                }
            }

            if (min_uv_x < 0.0f || min_uv_y < 0.0f || max_uv_x > 1.0f || max_uv_y > 1.0f) {
                if (min_uv_x < 0.0f) {
                    for (Vertex &v : misc) {
                        v.uv.x += min_uv_x * -1.0f;
                    }

                    max_uv_x += min_uv_x * -1.0f;
                }

                if (min_uv_y < 0.0f) {
                    for (Vertex &v : misc) {
                        v.uv.y += min_uv_y * -1.0f;
                    }

                    max_uv_y += min_uv_y * -1.0f;
                }

                for (Vertex &v : misc) {
                    v.uv.x /= max_uv_x;
                    v.uv.y /= max_uv_y;
                }
            }

        } else {
            throw std::runtime_error(importer.GetErrorString());
        }
    }
}