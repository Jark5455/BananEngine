//
// Created by yashr on 1/2/22.
//

#pragma once

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
        glm::vec3 color{1.f};
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

            std::shared_ptr<BananModel> model{};
            TransformComponent transform{};
            ParallaxComponent parallax{};

            std::string albedoalias;
            std::string normalalias;
            std::string heightalias;

            std::unique_ptr<PointLightComponent> pointLight = nullptr;

        private:
            BananGameObject(id_t objId, const BananGameObjectManager &manager);
            id_t id;

            const BananGameObjectManager &gameObjectManager;
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
            BananGameObject &makePointLight(float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));
            BananGameObject &duplicateGameObject(id_t index);

            BananGameObject &getGameObjectAtIndex(id_t index);

            size_t getImageIndexFromAlias(std::string alias);
            size_t getModelIndexFromAlias(std::string alias);

        private:
            std::shared_ptr<BananImage> loadImage(const std::string& filepath);
            std::shared_ptr<BananModel> loadMesh(const std::string& filepath);

            std::unordered_map<std::string, size_t> texturealias;
            std::vector<std::shared_ptr<BananImage>> textures;
            std::unordered_map<std::string, size_t> modelalias;
            std::vector<std::shared_ptr<BananModel>> models;

            BananGameObject::Map gameObjects;
            BananGameObject::id_t currentId;

            BananDevice &bananDevice;


            friend class BananGameObject;
    };
}