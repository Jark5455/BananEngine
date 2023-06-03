//
// Created by yashr on 1/2/22.
//

#pragma once

#include "banan_model.h"
#include "banan_swap_chain.h"
#include "banan_descriptor.h"

#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <unordered_map>

namespace Banan {
    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{1.f, 1.f, 1.f};
        glm::vec3 rotation{};

        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };

    struct TransformBuffer {
        alignas(16) glm::mat4 modelMatrix{1.f};
        alignas(16) glm::mat4 normalMatrix{1.f};
    };

    struct ParallaxComponent {
        alignas(4) float heightscale = -1.f;
        alignas(4) float parallaxBias = -1.f;
        alignas(4) float numLayers = -1.f;
        alignas(4) int parallaxmode  = -1;
    };

    struct PointLightBuffer {
        // TODO, fix position and radius, make part of transform component or something
        alignas(16) glm::vec4 position{1.f};
        alignas(16) glm::vec4 color{1.f};
        alignas(4) float radius = 1.0f;
        alignas(4) float lightIntensity = 1.0f;
        alignas(4) int shadowMapIndex;

        alignas(4) int hasNext = 0;
        alignas(8) uint64_t next = 0;
    };

    struct PointLightComponent {
        glm::vec3 color{1.f};
        float intensity = 1.0f;

        bool castsShadows = true;
        int shadowMapIndex;
    };

    struct GameObjectData {
        alignas(4) int albedoTexture = -1;
        alignas(4) int normalTexture = -1;
        alignas(4) int heightTexture = -1;

        alignas(4) int transform = 0;
        alignas(8) uint64_t transformRef = 0;

        alignas(4) int parallax = 0;
        alignas(8) uint64_t parallaxRef = 0;

        alignas(4) int pointLight = 0;
        alignas(8) uint64_t pointLightRef = 0;
    };

    class BananGameObjectManager;

    class BananGameObject {
        public:

            struct Builder {
                std::string modelPath;
                std::string albedoPath;
                std::string normalPath;
                std::string heightPath;
            };

            using id_t = unsigned int;
            using Map = std::unordered_map<id_t, BananGameObject>;

            BananGameObject(const BananGameObject &) = delete;
            BananGameObject &operator=(const BananGameObject &) = delete;
            BananGameObject(BananGameObject &&) = default;
            BananGameObject &operator=(BananGameObject &&) = delete;

            id_t getId();

            TransformComponent transform{};
            ParallaxComponent parallax{};

            std::string albedoalias;
            std::string normalalias;
            std::string heightalias;

            std::shared_ptr<BananModel> model = nullptr;
            std::unique_ptr<PointLightComponent> pointLight = nullptr;

        private:
            BananGameObject(id_t objId, const BananGameObjectManager &manager);
            id_t id;

            friend class BananGameObjectManager;
    };

    class BananGameObjectManager {
        public:
            BananGameObjectManager(BananDevice &device);

            BananGameObjectManager(const BananGameObjectManager &) = delete;
            BananGameObjectManager &operator=(const BananGameObjectManager &) = delete;
            BananGameObjectManager(BananGameObjectManager &&) = default;
            BananGameObjectManager &operator=(BananGameObjectManager &&) = delete;

            BananGameObject &makeVirtualGameObject();
            BananGameObject &makeGameObject(const BananGameObject::Builder &builder);
            BananGameObject &makePointLight(float intensity = 10.0f, float radius = 0.1f, glm::vec4 color = glm::vec4(1.f));
            BananGameObject &duplicateGameObject(BananGameObject::id_t index);
            BananGameObject &copyGameObject(BananGameObject::id_t indexOld, BananGameObject::id_t indexNew);

            BananGameObject &getGameObjectAtIndex(BananGameObject::id_t index);
            BananGameObject::Map &getGameObjects();
            size_t getGameObjectBufferAlignmentSize(size_t frameIndex);

            size_t getImageIndexFromAlias(std::string alias);
            size_t getModelIndexFromAlias(std::string alias);

            size_t numTextures();
            std::vector<VkDescriptorImageInfo> textureInfo();

            size_t numPointLights();
            uint64_t getPointLightBaseRef(size_t frameIndex);

            VkDescriptorSetLayout getGameObjectSetLayout();
            VkDescriptorSetLayout getTextureSetLayout();

            VkDescriptorSet &getGameObjectDescriptorSet(size_t frameIndex);
            VkDescriptorSet &getTextureDescriptorSet(size_t frameIndex);

            void updateBuffers(size_t frameIndex);
            void createBuffers(size_t preAllocatedGameObjects);
            void buildDescriptors();

        private:
            std::shared_ptr<BananImage> loadImage(const std::string& filepath);
            std::shared_ptr<BananModel> loadMesh(const std::string& filepath);
            PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;

            std::unordered_map<std::string, size_t> texturealias;
            std::vector<std::shared_ptr<BananImage>> textures;
            std::unordered_map<std::string, size_t> modelalias;
            std::vector<std::shared_ptr<BananModel>> models;

            std::vector<std::unique_ptr<BananBuffer>> gameObjectDataBuffers{BananSwapChain::MAX_FRAMES_IN_FLIGHT};
            std::vector<std::unique_ptr<BananBuffer>> transformBuffers{BananSwapChain::MAX_FRAMES_IN_FLIGHT};
            std::vector<std::unique_ptr<BananBuffer>> parallaxBuffers{BananSwapChain::MAX_FRAMES_IN_FLIGHT};
            std::vector<std::unique_ptr<BananBuffer>> pointLightBuffers{BananSwapChain::MAX_FRAMES_IN_FLIGHT};

            BananGameObject::Map gameObjects;
            BananGameObject::id_t currentId;

            std::unique_ptr<BananDescriptorPool> gameObjectPool;

            std::vector<VkDescriptorSet> gameObjectDataDescriptorSets{BananSwapChain::MAX_FRAMES_IN_FLIGHT};
            std::vector<VkDescriptorSet> textureDescriptorSets{BananSwapChain::MAX_FRAMES_IN_FLIGHT};

            std::unique_ptr<BananDescriptorSetLayout> gameObjectDataSetLayout = nullptr;
            std::unique_ptr<BananDescriptorSetLayout> textureSetLayout = nullptr;

            BananDevice &bananDevice;

            friend class BananGameObject;
    };
}
