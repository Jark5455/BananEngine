//
// Created by yashr on 1/2/22.
//

#pragma once

#include "banan_swap_chain.h"
#include "banan_model.h"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace Banan {

    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{1.f, 1.f, 1.f};
        glm::vec3 rotation{};
        int id;

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
        float lightIntensity = 1.0f;
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
        int parallaxmode = 1;
    };

    class BananGameObjectManager;

    class BananGameObject {
        public:
            using id_t = unsigned int;
            using Map = std::unordered_map<id_t, BananGameObject>;

            static BananGameObject createGameObject();
            static BananGameObject makePointLight(float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

            BananGameObject(BananGameObject &&) = default;
            BananGameObject(const BananGameObject &) = delete;
            BananGameObject &operator=(const BananGameObject &) = delete;
            BananGameObject &operator=(BananGameObject &&) = delete;

            id_t getId();

            GameObjectData getObjectData(int frameIndex);

            glm::vec3 color{};
            TransformComponent transform{};
            ParallaxComponent parallax{};

            std::shared_ptr<BananImage> texture;
            std::shared_ptr<BananImage> normal;
            std::shared_ptr<BananImage> height;

            std::shared_ptr<BananModel> model{};
            std::unique_ptr<PointLightComponent> pointLight = nullptr;

        private:
            BananGameObject(id_t objId, const BananGameObjectManager &manager);

            id_t id;
            const BananGameObjectManager &gameObjectManger;

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
            VkDescriptorBufferInfo getUboBufferInfo();

            void updateBuffer(int frameIndex);

            BananGameObject::Map gameObjects{};
            std::vector<std::unique_ptr<BananBuffer>> uboBuffers{BananSwapChain::MAX_FRAMES_IN_FLIGHT};

        private:
            BananGameObject::id_t currentId = 0;
    };
}