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
    };

    class BananGameObject {
        public:
            using id_t = unsigned int;
            using Map = std::unordered_map<id_t, BananGameObject>;

            static BananGameObject createGameObject();
            static BananGameObject makePointLight(float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

            BananGameObject(const BananGameObject &) = delete;
            BananGameObject &operator=(const BananGameObject &) = delete;
            BananGameObject(BananGameObject &&) = default;
            BananGameObject &operator=(BananGameObject &&) = default;

            id_t getId();

            std::shared_ptr<BananModel> model{};
            glm::vec3 color{};
            TransformComponent transform{};
            ParallaxComponent parallax{};

            std::unique_ptr<PointLightComponent> pointLight = nullptr;

        private:
            BananGameObject(id_t objId) : id{objId} {}
            const id_t id;
    };
}