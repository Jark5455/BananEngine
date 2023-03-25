//
// Created by yashr on 1/2/22.
//

#include "banan_game_object.h"

#include <stb_image.h>

#include <openexr.h>

#include <ImathBox.h>
#include <ImfRgbaFile.h>
#include <ImfArray.h>

#include <thread>

namespace Banan {
    BananGameObject::BananGameObject(id_t objId, const BananGameObjectManager &manager) : gameObjectManager{manager} {
        id = objId;
    }

    BananGameObject::id_t BananGameObject::getId() {
        return id;
    }

    BananGameObject BananGameObject::makePointLight(float intensity, float radius, glm::vec3 color) {
        BananGameObject obj = BananGameObject::createGameObject();
        obj.pointLight->color = color;
        obj.transform.scale.x = radius;
        obj.pointLight = std::make_unique<PointLightComponent>();
        obj.pointLight->lightIntensity = intensity;
        return obj;
    }

    glm::mat4 TransformComponent::mat4() {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);

        return glm::mat4{
                {
                        scale.x * (c1 * c3 + s1 * s2 * s3),
                        scale.x * (c2 * s3),
                        scale.x * (c1 * s2 * s3 - c3 * s1),
                        0.0f
                },
                {
                        scale.y * (c3 * s1 * s2 - c1 * s3),
                        scale.y * (c2 * c3),
                        scale.y * (c1 * c3 * s2 + s1 * s3),
                        0.0f
                },
                {
                        scale.z * (c2 * s1),
                        scale.z * (-s2),
                        scale.z * (c1 * c2),
                        0.0f
                },
                {
                    translation.x,
                    translation.y,
                    translation.z,
                    1
                }
        };
    }

    glm::mat3 TransformComponent::normalMatrix() {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        const glm::vec3 invScale = 1.0f / scale;

        return glm::mat3{
                {
                        invScale.x * (c1 * c3 + s1 * s2 * s3),
                        invScale.x * (c2 * s3),
                        invScale.x * (c1 * s2 * s3 - c3 * s1)
                },
                {
                        invScale.y * (c3 * s1 * s2 - c1 * s3),
                        invScale.y * (c2 * c3),
                        invScale.y * (c1 * c3 * s2 + s1 * s3)
                },
                {
                        invScale.z * (c2 * s1),
                        invScale.z * (-s2),
                        invScale.z * (c1 * c2)
                }
        };
    }

    BananGameObjectManager::BananGameObjectManager(BananDevice &device) : bananDevice{device} {

    }

    BananGameObject &BananGameObjectManager::makeGameObject(BananGameObjectBuilder builder) {
        auto gameObject = BananGameObject{currentId++, *this};
        auto gameObjectId = gameObject.getId();
        gameObjects.emplace(gameObjectId, std::move(gameObject));
        return gameObjects.at(gameObjectId);
    }

    size_t BananGameObjectManager::loadImage(TextureType type, std::string filepath) {char *fileName = const_cast<char *>(filepath.c_str());
        size_t len = strlen(fileName);
        size_t idx = len-1;
        for(size_t i = 0; *(fileName+i); i++) {
            if (*(fileName+i) == '.') {
                idx = i;
            } else if (*(fileName + i) == '/' || *(fileName + i) == '\\') {
                idx = len - 1;
            }
        }

        std::string extension = std::string(fileName).substr(idx+1);

        assert(
                extension == "exr" ||
                extension == "jpg" ||
                extension == "jpeg" ||
                extension == "png" &&
                "File format not supported");

        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t stride = 0;
        uint32_t mipLevels = 0;
        void *data;

        if (extension == "exr") {
            Imf::setGlobalThreadCount((int) std::thread::hardware_concurrency());
            Imf::Array2D<Imf::Rgba> pixelBuffer = Imf::Array2D<Imf::Rgba>();
            Imf::Array2D<Imf::Rgba> &pixelBufferRef = pixelBuffer;

            Imf::RgbaInputFile in(filepath.c_str());
            Imath::Box2i win = in.dataWindow();
            Imath::V2i dim(win.max.x - win.min.x + 1, win.max.y - win.min.y + 1);

            int dx = win.min.x;
            int dy = win.min.y;

            pixelBufferRef.resizeErase(dim.x, dim.y);

            in.setFrameBuffer(&pixelBufferRef[0][0] - dx - dy * dim.x, 1, dim.x);
            in.readPixels(win.min.y, win.max.y);

            width = dim.x;
            height = dim.y;
            stride = 16;
            mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, width)))) + 1;

            std::vector<uint16_t> singleChannelPixelBuffer{};
            singleChannelPixelBuffer.reserve(dim.y * dim.x);

            int index = 0;

            for (int y1 = 0; y1 < dim.y; y1++) {
                for (int x1 = 0; x1 < dim.x; x1++) {
                    singleChannelPixelBuffer[index++] = pixelBufferRef[y1][x1].r.bits();
                    singleChannelPixelBuffer[index++] = pixelBufferRef[y1][x1].g.bits();
                    singleChannelPixelBuffer[index++] = pixelBufferRef[y1][x1].b.bits();
                    singleChannelPixelBuffer[index++] = pixelBufferRef[y1][x1].a.bits();
                }
            }

            data = malloc(dim.x * dim.y * 8);
            memcpy(data, singleChannelPixelBuffer.data(), dim.y * dim.x * 8);

        } else {
            void* otherdata = stbi_load(filepath.c_str(), (int *) &width, (int *) &height, nullptr, STBI_rgb_alpha);
            mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, width)))) + 1;
            stride = 8;

            data = malloc(width * height * 4);
            memcpy(data, otherdata, width * height * 4);

            stbi_image_free(otherdata);
        }

        assert(width != 0 && height != 0 && data != nullptr && "something went wrong when loading textures");

        uint32_t pixelCount = height * width;
        uint32_t pixelSize = stride / 2; // stride is in bits, pixel size should be in bytes

        VkFormat format = stride == 16 ? VK_FORMAT_R16G16B16A16_UNORM : VK_FORMAT_R8G8B8A8_UNORM;

        BananBuffer stagingBuffer{bananDevice, pixelSize, pixelCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *) data);
        free(data);

        VkCommandBuffer commandBuffer = bananDevice.beginSingleTimeCommands();

        auto image = std::make_shared<BananImage>(bananDevice, width, height, mipLevels, format, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        image->transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        bananDevice.endSingleTimeCommands(commandBuffer);
        bananDevice.copyBufferToImage(stagingBuffer.getBuffer(), image->getImageHandle(), width, height, 1);
        bananDevice.generateMipMaps(image->getImageHandle(), width, height, mipLevels);

        textures.push_back(image);
        return textures.size() - 1;
    }
}
