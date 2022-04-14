//
// Created by yashr on 12/30/21.
//

#include "banan_model.h"
#include "banan_logger.h"

#include <cassert>
#include <cstring>
#include <iterator>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>

namespace Banan {
    BananModel::BananModel(BananDevice &device, const Builder &builder) : bananDevice(device) {
        createTextureImage(builder.texture);
        createVertexBuffers(builder.vertices);
        createIndexBuffers(builder.indices);
    }

    BananModel::~BananModel() {}

    void BananModel::bind(VkCommandBuffer commandBuffer) {
        VkBuffer buffers[] = {vertexBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

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

    void BananModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be atleast 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        BananBuffer stagingBuffer{bananDevice, vertexSize, vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *)vertices.data());

        vertexBuffer = std::make_unique<BananBuffer>(bananDevice, vertexSize, vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        bananDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
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

    std::unique_ptr<BananModel> BananModel::createModelFromFile(BananDevice &device, const std::string &filepath) {
        Builder builder{};
        builder.loadModel(filepath);
        LogInfo() << "Model Vertex Count: " + std::to_string(builder.vertices.size());
        return std::make_unique<BananModel>(device, builder);
    }

    void BananModel::createTextureImage(const Texture &texture) {
        if (texture.width > 0 && texture.height > 0) {
            pixelCount = texture.width * texture.height;
            assert(pixelCount >= 0 && "Failed to load image: 0 pixels");
            uint32_t pixelSize = sizeof(unsigned char);

            BananBuffer stagingBuffer{bananDevice, pixelSize, pixelCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
            stagingBuffer.map();
            stagingBuffer.writeToBuffer((void *)texture.data);

            textureImage = std::make_unique<BananImage>(bananDevice, pixelSize, texture.height, texture.width, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            bananDevice.transitionImageLayout(textureImage->getImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            bananDevice.copyBufferToImage(stagingBuffer.getBuffer(), textureImage->getImage(), texture.width, texture.height, 1);
            bananDevice.transitionImageLayout(textureImage->getImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    }

    std::vector<VkVertexInputBindingDescription> BananModel::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> BananModel::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
        attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
        attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
        attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

        return attributeDescriptions;
    }

    void BananModel::Builder::loadModel(const std::string &filepath) {
        Assimp::Importer importer;
        const aiScene *scene = importer.ReadFile(filepath,aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

        vertices.clear();
        indices.clear();

        if (scene) {
            for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
                const aiMesh *mesh = scene->mMeshes[i];
                for (uint32_t j = 0; j < mesh->mNumVertices; j++) {
                    Vertex v{};
                    v.position = glm::vec3{ mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z};
                    v.normal = glm::vec3{mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z};

                    v.color =  mesh->HasVertexColors(j) ? glm::vec3{mesh->mColors[j]->r, mesh->mColors[j]->g, mesh->mColors[j]->b} : glm::vec3{1.0f, 1.0f, 1.0f};
                    v.uv = mesh->HasTextureCoords(j) ? glm::vec2{mesh->mTextureCoords[j]->x, mesh->mTextureCoords[j]->y} : glm::vec2{0.0f, 0.0f};
                    vertices.push_back(v);
                }

                for (uint32_t k = 0; k < mesh->mNumFaces; k++) {
                    indices.push_back(mesh->mFaces[k].mIndices[0]);
                    indices.push_back(mesh->mFaces[k].mIndices[1]);
                    indices.push_back(mesh->mFaces[k].mIndices[2]);
                }
            }

            if (scene->HasTextures()) {
                if (scene->mNumTextures == 1) {
                    const aiTexture *sceneTexture = scene->mTextures[0];

                    int width = 0;
                    int height = 0;
                    int texChannels = 0;

                    if (sceneTexture->mHeight == 0)
                    {
                        texture.data = stbi_load_from_memory(reinterpret_cast<uint8_t *>(sceneTexture->pcData), static_cast<int>(sceneTexture->mWidth), &width, &height, &texChannels, STBI_rgb_alpha);
                    }
                    else
                    {
                        texture.data = stbi_load_from_memory(reinterpret_cast<uint8_t *>(sceneTexture->pcData), static_cast<int>(sceneTexture->mWidth * sceneTexture->mHeight), &width, &height, &texChannels, STBI_rgb_alpha);
                    }

                    texture.width = width;
                    texture.height = height;
                }
            }
        } else {
            throw std::runtime_error(importer.GetErrorString());
        }
    }

    void BananModel::Builder::loadTexture(const std::string &filepath) {
        int texWidth, texHeight, texChannels;
        uint8_t *data = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        texture.data = data;
        texture.width = texWidth;
        texture.height = texHeight;
    }
}
