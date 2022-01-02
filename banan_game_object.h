//
// Created by yashr on 1/2/22.
//

#pragma once

#include "banan_model.h"

#include <memory>

namespace Banan {

    struct Transform2dComponent {
        glm::vec2 translation{};
        glm::vec2 scale{1.f, 1.f};
        float rotation;

        glm::mat2 mat2();
    };

    class BananGameObject {
        public:
            using id_t = unsigned int;

            static BananGameObject createGameObject();

            BananGameObject(const BananGameObject &) = delete;
            BananGameObject &operator=(const BananGameObject &) = delete;
            BananGameObject(BananGameObject&&) = default;
            BananGameObject &operator=(BananGameObject &&) = default;

            id_t getId();

            std::shared_ptr<BananModel> model{};
            glm::vec3 color{};
            Transform2dComponent transform2D{};

        private:
            BananGameObject(id_t objId) : id{objId} {}
            const id_t id;
    };
}