//
// Created by yashr on 1/2/22.
//

#pragma once

#include "banan_model.h"

#include <glm/gtc/matrix_transform.hpp>
#include <memory>

namespace Banan {

    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{1.f, 1.f, 1.f};
        glm::vec3 rotation{};

        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };

    class BananGameObject {
        public:
            using id_t = unsigned int;

            static BananGameObject createGameObject();

            BananGameObject(const BananGameObject &) = delete;
            BananGameObject &operator=(const BananGameObject &) = delete;
            BananGameObject(BananGameObject &&) = default;
            BananGameObject &operator=(BananGameObject &&) = default;

            id_t getId();

            std::shared_ptr<BananModel> model{};
            glm::vec3 color{};
            TransformComponent transform{};

        private:
            BananGameObject(id_t objId) : id{objId} {}
            const id_t id;
    };
}