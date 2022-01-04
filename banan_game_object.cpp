//
// Created by yashr on 1/2/22.
//

#include "banan_game_object.h"

Banan::BananGameObject Banan::BananGameObject::createGameObject() {
    static id_t currentId = 0;
    return {currentId++};
}

Banan::BananGameObject::id_t Banan::BananGameObject::getId() {
    return id;
}

glm::mat4 Banan::TransformComponent::mat4() {
    auto transform = glm::translate(glm::mat4{1.f}, translation);

    transform = glm::rotate(transform, rotation.y, {0.f, 1.f, 0.f});
    transform = glm::rotate(transform, rotation.x, {1.f, 0.f, 0.f});
    transform = glm::rotate(transform, rotation.z, {0.f, 0.f, 1.f});
    transform = glm::scale(transform, scale);

    transform = glm::scale(transform, scale);
    return transform;
}
