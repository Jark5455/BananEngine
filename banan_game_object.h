//
// Created by yashr on 1/2/22.
//

#pragma once

#include "banan_camera.h"
#include "banan_swap_chain.h"
#include "banan_model.h"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>
#include <tuple>

namespace Banan {

    enum TextureType {
        BANAN_TEXTURE_TYPE_COLOR,
        BANAN_TEXTURE_TYPE_NORMAL,
        BANAN_TEXTURE_TYPE_HEIGHT,
        BANAN_TEXTURE_TYPE_ROUGHNESS,
        BANAN_TEXTURE_TYPE_METAL
    };

    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{1.f, 1.f, 1.f};
        glm::vec3 rotation{};
        id_t id;

        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };

    struct ParallaxComponent {
        float heightscale = -1.f;
        float parallaxBias = -1.f;
        float numLayers = -1.f;
        int parallaxmode  = -1;
    };

    struct PointLightComponent {
        glm::vec3 color{1.f};
        float lightIntensity = 1.0f;

        std::unique_ptr<BananCubemap> shadowMap;
    };

    struct TextureComponent {
        int colorLocation = -1;
        int normalLocation = -1;
        int heightLocation = -1;
    };

    struct GameObjectData {
        alignas(16) glm::mat4 modelMatrix{1.f};
        alignas(16) glm::mat4 normalMatrix{1.f};

        int textureLocation = -1;
        int normalLocation = -1;
        int heightLocation = -1;

        float heightscale = 0.1;
        float parallaxBias = -0.02f;
        float numLayers = 48.0f;
        int parallaxmode = -1;
    };

    class BananGameObjectManager;

    class BananGameObject {
        public:
            using id_t = unsigned int;
            using Map = std::unordered_map<id_t, BananGameObject>;

            BananGameObject(BananGameObject &&) = default;
            BananGameObject(const BananGameObject &) = delete;
            BananGameObject &operator=(const BananGameObject &) = delete;
            BananGameObject &operator=(BananGameObject &&) = delete;

            id_t getId();
            GameObjectData& getObjectData(int frameIndex);

            TransformComponent transform{};
            ParallaxComponent parallax{};

            void loadTexture(TextureType type, std::string filepath, std::string name);
            void loadModel(std::string filepath, std::string name);

            std::tuple<std::string, std::string, std::shared_ptr<BananImage>> &colorMap;
            std::tuple<std::string, std::string, std::shared_ptr<BananImage>> &normalMap;
            std::tuple<std::string, std::string, std::shared_ptr<BananImage>> &heightMap;

            std::tuple<std::string, std::string, std::shared_ptr<BananModel>> &model;

            std::shared_ptr<PointLightComponent> pointLight = nullptr;
            std::shared_ptr<BananCamera> camera = nullptr;

        private:
            BananGameObject(id_t objId, const BananGameObjectManager &manager);

            id_t id;
            BananGameObjectManager &gameObjectManager;
            GameObjectData gameObjectData{};

            friend class BananGameObjectManager;
    };

    class BananGameObjectManager {
        public:
            BananGameObjectManager(BananDevice &device);
            BananGameObjectManager(const BananGameObjectManager &) = delete;
            BananGameObjectManager &operator=(const BananGameObjectManager &) = delete;
            BananGameObjectManager(BananGameObjectManager &&) = delete;
            BananGameObjectManager &operator=(BananGameObjectManager &&) = delete;

            BananGameObject& createGameObject();
            BananGameObject &makePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

            VkDescriptorBufferInfo getUboBufferInfo(int frameIndex);
            std::vector<VkDescriptorImageInfo> getTextureInfo();

            void updateBuffer(int frameIndex);

            BananGameObject::Map gameObjects{};

            std::vector<std::shared_ptr<BananBuffer>> transformBuffer{BananSwapChain::MAX_FRAMES_IN_FLIGHT};
            std::vector<std::shared_ptr<BananBuffer>> parallaxBuffer{BananSwapChain::MAX_FRAMES_IN_FLIGHT};
            std::vector<std::shared_ptr<BananBuffer>> pointLightBuffer{BananSwapChain::MAX_FRAMES_IN_FLIGHT};
            std::vector<std::shared_ptr<BananBuffer>> textureBuffer{BananSwapChain::MAX_FRAMES_IN_FLIGHT};

            std::vector<std::tuple<std::string, std::string, std::shared_ptr<BananModel>>> models;
            std::vector<std::tuple<std::string, std::string, std::shared_ptr<BananImage>>> textures;

        private:
            void createUboBuffers(uint32_t count);

            void addTexture(const std::tuple<std::string, std::string, std::shared_ptr<BananImage>> &t);
            void addModel(const std::tuple<std::string, std::string, std::shared_ptr<BananModel>> &m);

            BananGameObject::id_t currentId = 0;
            BananDevice& bananDevice;

            friend class BananGameObject;
    };
}