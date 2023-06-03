//
// Created by yashr on 1/2/22.
//

#include "banan_game_object.h"

#include <algorithm>
#include <iostream>

namespace Banan {
    BananGameObject::BananGameObject(id_t objId, const BananGameObjectManager &manager) {
        id = objId;
    }

    BananGameObject::id_t BananGameObject::getId() {
        return id;
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
        vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(bananDevice.device(), "vkGetBufferDeviceAddressKHR"));
        currentId = 0;
    }

    void BananGameObjectManager::buildDescriptors() {
        gameObjectPool = BananDescriptorPool::Builder(bananDevice)
                .setMaxSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT * 2)
                .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, BananSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, BananSwapChain::MAX_FRAMES_IN_FLIGHT * static_cast<uint32_t>(numTextures()))
                .build();

        textureSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addFlag(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT)
                .addFlag(VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT)
                .addFlag(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT)
                .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, static_cast<uint32_t>(numTextures()))
                .build();

        gameObjectDataSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_ALL_GRAPHICS)
                .build();

        for (auto &set : textureDescriptorSets) {
            BananDescriptorWriter writer = BananDescriptorWriter(*textureSetLayout, *gameObjectPool);
            auto info = textureInfo();
            writer.writeImages(0, info);
            writer.build(set, {static_cast<uint32_t>(numTextures())});
        }

        for (size_t i = 0; i < gameObjectDataDescriptorSets.size(); i++) {
            BananDescriptorWriter writer = BananDescriptorWriter(*gameObjectDataSetLayout, *gameObjectPool);
            auto info = gameObjectDataBuffers[i]->descriptorInfo(gameObjectDataBuffers[i]->getInstanceSize());
            writer.writeBuffer(0, info);
            writer.build(gameObjectDataDescriptorSets[i]);
        }
    }

    void BananGameObjectManager::createBuffers(size_t preAllocatedGameObjects) {
        for (auto& buffer : gameObjectDataBuffers) {
            buffer = std::make_unique<BananBuffer>(bananDevice, sizeof(GameObjectData), gameObjects.size() + preAllocatedGameObjects, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, bananDevice.physicalDeviceProperties().limits.minUniformBufferOffsetAlignment);
            buffer->map();
        }

        //TODO replace this, counts are inefficient

        size_t transformCount = 0;
        size_t parallaxCount = 0;
        size_t pointLightCount = 0;

        for (auto &kv : gameObjects) {
            if (kv.second.model != nullptr) transformCount++;
            if (kv.second.pointLight != nullptr) pointLightCount++;
            if (kv.second.parallax.parallaxmode != -1) parallaxCount++;
        }

        for (auto& buffer : transformBuffers) {
            buffer = std::make_unique<BananBuffer>(bananDevice, sizeof(TransformBuffer), transformCount, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0);
            buffer->map();
        }

        for (auto& buffer : parallaxBuffers) {
            buffer = std::make_unique<BananBuffer>(bananDevice, sizeof(ParallaxComponent), parallaxCount, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0);
            buffer->map();
        }

        for (auto& buffer : pointLightBuffers) {
            buffer = std::make_unique<BananBuffer>(bananDevice, sizeof(PointLightBuffer), pointLightCount, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, 0);
            buffer->map();
        }

        // TODO reconstruct pipelines if this is a resizing of buffers
    }

    void BananGameObjectManager::updateBuffers(size_t frameIndex) {
        VkBufferDeviceAddressInfo address_info{};
        address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;

        address_info.buffer = transformBuffers[frameIndex]->getBuffer();
        uint64_t transformBufferAddr = vkGetBufferDeviceAddressKHR(bananDevice.device(), &address_info);
        size_t transformCount = 0;

        address_info.buffer = parallaxBuffers[frameIndex]->getBuffer();
        uint64_t parallaxBufferAddr = vkGetBufferDeviceAddressKHR(bananDevice.device(), &address_info);
        size_t parallaxCount = 0;

        address_info.buffer = pointLightBuffers[frameIndex]->getBuffer();
        uint64_t pointLightBufferAddr = vkGetBufferDeviceAddressKHR(bananDevice.device(), &address_info);
        size_t pointLightCount = 0;

        for (auto &kv : gameObjects) {
            GameObjectData data{};

            if (kv.second.model != nullptr) {
                data.transform = 1;
                data.transformRef = transformBufferAddr + (transformCount * transformBuffers[frameIndex]->getAlignmentSize());

                TransformBuffer transform{};
                transform.modelMatrix = kv.second.transform.mat4();
                transform.normalMatrix = kv.second.transform.normalMatrix();

                assert(transformCount < transformBuffers[frameIndex]->getInstanceCount() && "ubo too small");
                transformBuffers[frameIndex]->writeToIndex(&transform, transformCount);
                transformBuffers[frameIndex]->flushIndex(transformCount++);
            }

            if (kv.second.pointLight != nullptr) {
                data.pointLight = 1;
                data.pointLightRef = pointLightBufferAddr + (pointLightCount * pointLightBuffers[frameIndex]->getAlignmentSize());

                assert(pointLightCount < pointLightBuffers[frameIndex]->getInstanceCount() && "ubo too small");

                PointLightBuffer pointlight{};
                pointlight.position = glm::vec4{kv.second.transform.translation, 1.f};
                pointlight.radius = kv.second.transform.scale.x;
                pointlight.color = glm::vec4{kv.second.pointLight->color, 1.f};
                pointlight.lightIntensity = kv.second.pointLight->intensity;
                pointlight.shadowMapIndex = kv.second.pointLight->castsShadows ? kv.second.pointLight->shadowMapIndex : -1;

                pointlight.hasNext = 1;
                pointlight.next = data.pointLightRef + pointLightBuffers[frameIndex]->getAlignmentSize();

                pointLightBuffers[frameIndex]->writeToIndex(&pointlight, pointLightCount);
                pointLightBuffers[frameIndex]->flushIndex(pointLightCount++);
            }

            if (kv.second.parallax.parallaxmode != -1) {
                data.parallax = 1;
                data.parallaxRef = parallaxBufferAddr + (parallaxCount * parallaxBuffers[frameIndex]->getAlignmentSize());

                assert(parallaxCount < parallaxBuffers[frameIndex]->getInstanceCount() && "ubo too small");
                parallaxBuffers[frameIndex]->writeToIndex(&kv.second.parallax, parallaxCount);
                parallaxBuffers[frameIndex]->flushIndex(parallaxCount++);
            }

            if (!kv.second.albedoalias.empty() && texturealias.find(kv.second.albedoalias) != texturealias.end()) {
                data.albedoTexture = static_cast<int>(texturealias.at(kv.second.albedoalias));
            }

            if (!kv.second.normalalias.empty() && texturealias.find(kv.second.normalalias) != texturealias.end()) {
                data.normalTexture = static_cast<int>(texturealias.at(kv.second.normalalias));
            }

            if (!kv.second.heightalias.empty() && texturealias.find(kv.second.heightalias) != texturealias.end()) {
                data.heightTexture = static_cast<int>(texturealias.at(kv.second.heightalias));
            }

            gameObjectDataBuffers[frameIndex]->writeToIndex(&data, kv.first);
            gameObjectDataBuffers[frameIndex]->flushIndex(kv.first);
        }

        // TODO unsafe pointer arithmetic, fix later
        if (pointLightCount > 0) {
            auto *data = static_cast<PointLightBuffer *>(pointLightBuffers[frameIndex]->readIndex(pointLightCount - 1));
            data->hasNext = 0;
            data->next = 0;
        }
    }

    BananGameObject &BananGameObjectManager::makeVirtualGameObject() {
        auto gameObject = BananGameObject{currentId++, *this};
        auto gameObjectId = gameObject.getId();
        gameObjects.emplace(gameObjectId, std::move(gameObject));

        // TODO buffer recreation if buffer too small

        return gameObjects.at(gameObjectId);
    }

    BananGameObject &BananGameObjectManager::makeGameObject(const BananGameObject::Builder &builder) {
        BananGameObject &gameObject = makeVirtualGameObject();

        if (!builder.modelPath.empty()) {
            gameObject.model = loadMesh(builder.modelPath);
        }

        if (!builder.albedoPath.empty()) {
            gameObject.albedoalias = builder.albedoPath;
            loadImage(builder.albedoPath);
        }

        if (!builder.normalPath.empty()) {
            gameObject.normalalias = builder.normalPath;
            loadImage(builder.normalPath);
        }

        if (!builder.heightPath.empty()) {
            gameObject.heightalias = builder.heightPath;
            loadImage(builder.heightPath);
        }

        return gameObject;
    }

    BananGameObject &BananGameObjectManager::makePointLight(float intensity, float radius, glm::vec4 color) {
        auto gameObject = BananGameObject{currentId++, *this};
        auto gameObjectId = gameObject.getId();
        gameObjects.emplace(gameObjectId, std::move(gameObject));

        gameObjects.at(gameObjectId).pointLight = std::make_unique<PointLightComponent>();
        gameObjects.at(gameObjectId).pointLight->color = color;
        gameObjects.at(gameObjectId).pointLight->intensity = intensity;
        gameObjects.at(gameObjectId).transform.scale.x = radius;
        return gameObjects.at(gameObjectId);
    }

    std::shared_ptr<BananImage> BananGameObjectManager::loadImage(const std::string& filepath) {
        if (texturealias.find(filepath) != texturealias.end())
            return textures.at(texturealias.at(filepath));

        auto image = BananImage::makeImageFromFilepath(bananDevice, filepath);
        textures.push_back(image);
        texturealias.emplace(filepath, textures.size() - 1);

        return image;
    }

    std::shared_ptr<BananModel> BananGameObjectManager::loadMesh(const std::string& filepath) {
        if (modelalias.find(filepath) != modelalias.end())
            return models.at(modelalias.at(filepath));

        auto model = BananModel::createModelFromFile(bananDevice, filepath);
        models.push_back(model);
        modelalias.emplace(filepath, textures.size() - 1);

        return model;
    }

    BananGameObject &BananGameObjectManager::copyGameObject(BananGameObject::id_t indexOld, BananGameObject::id_t indexNew) {
        auto &object = getGameObjectAtIndex(indexOld);
        auto &new_object = getGameObjectAtIndex(indexNew);

        if (object.pointLight != nullptr) {
            new_object.pointLight = std::make_unique<PointLightComponent>();
            new_object.pointLight->color = object.pointLight->color;
            new_object.pointLight->intensity = object.pointLight->intensity;

            new_object.transform = object.transform;
            return new_object;
        }


        new_object.transform = object.transform;
        new_object.parallax = object.parallax;

        new_object.albedoalias = object.albedoalias;
        new_object.normalalias = object.normalalias;
        new_object.heightalias = object.heightalias;

        new_object.model = object.model;
        return new_object;
    }

    BananGameObject &BananGameObjectManager::duplicateGameObject(BananGameObject::id_t index) {
        auto &new_object = makeVirtualGameObject();
        copyGameObject(index, new_object.getId());
        return new_object;
    }

    std::vector<VkDescriptorImageInfo> BananGameObjectManager::textureInfo() {
        std::vector<VkDescriptorImageInfo> info{numTextures()};
        for (size_t i = 0; i < textures.size(); i++) {
            info[i] = textures[i]->descriptorInfo();
        }

        return info;
    }

    BananGameObject &BananGameObjectManager::getGameObjectAtIndex(BananGameObject::id_t index) {
        return gameObjects.at(index);
    }

    BananGameObject::Map &BananGameObjectManager::getGameObjects() {
        return gameObjects;
    }

    size_t BananGameObjectManager::getImageIndexFromAlias(std::string alias) {
        return texturealias.at(alias);
    }

    size_t BananGameObjectManager::getModelIndexFromAlias(std::string alias) {
        return modelalias.at(alias);
    }

    size_t BananGameObjectManager::numTextures() {
        return textures.size();
    }

    VkDescriptorSetLayout BananGameObjectManager::getGameObjectSetLayout() {
        return gameObjectDataSetLayout->getDescriptorSetLayout();
    }

    VkDescriptorSetLayout BananGameObjectManager::getTextureSetLayout() {
        return textureSetLayout->getDescriptorSetLayout();
    }

    VkDescriptorSet &BananGameObjectManager::getGameObjectDescriptorSet(size_t frameIndex) {
        return gameObjectDataDescriptorSets[frameIndex];
    }

    VkDescriptorSet &BananGameObjectManager::getTextureDescriptorSet(size_t frameIndex) {
        return textureDescriptorSets[frameIndex];
    }

    size_t BananGameObjectManager::numPointLights() {
        size_t count = 0;
        for (auto &kv : gameObjects) {
            if (kv.second.pointLight != nullptr)
                count++;
        }

        return count;
    }

    uint64_t BananGameObjectManager::getPointLightBaseRef(size_t frameIndex) {
        VkBufferDeviceAddressInfo address_info{};
        address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        address_info.buffer = pointLightBuffers[frameIndex]->getBuffer();

        return vkGetBufferDeviceAddressKHR(bananDevice.device(), &address_info);
    }

    size_t BananGameObjectManager::getGameObjectBufferAlignmentSize(size_t frameIndex) {
        return gameObjectDataBuffers[frameIndex]->getAlignmentSize();
    }
}
