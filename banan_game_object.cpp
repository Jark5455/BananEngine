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

glm::mat2 Banan::Transform2dComponent::mat2() {
    const float s = glm::sin(rotation);
    const float c = glm::cos(rotation);
    glm::mat2 rotMatrix{{c, s}, {-s, c}};

    glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};
    return rotMatrix * scaleMat;
}
