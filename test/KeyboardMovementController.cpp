//
// Created by yashr on 1/8/22.
//

#include "KeyboardMovementController.h"

#include <limits>

namespace BananTest {
    void KeyboardMovementController::moveInPlaneXZ(float dt, Banan::BananGameObject &object) {
        glm::vec3 rotate{0.f};
        glm::vec3 moveDir{0.f};

        SDL_PumpEvents();
        const Uint8 *keyboardState = SDL_GetKeyboardState(nullptr);

        if (keyboardState[keys.lookRight]) {
            rotate.y += 1.f;
        }

        if (keyboardState[keys.lookLeft]) {
            rotate.y -= 1.f;
        }

        if (keyboardState[keys.lookUp]) {
            rotate.x += 1.f;
        }

        if (keyboardState[keys.lookDown]) {
            rotate.x -= 1.f;
        }

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            object.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
        }

        object.transform.rotation.x = glm::clamp(object.transform.rotation.x, -1.5f, 1.5f);
        object.transform.rotation.y = glm::mod(object.transform.rotation.y, glm::two_pi<float>());

        float yaw = object.transform.rotation.y;
        const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
        const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
        const glm::vec3 upDir{0.f, -1.f, 0.f};

        if (keyboardState[keys.moveForward]) {
            moveDir += forwardDir;
        }

        if (keyboardState[keys.moveBackward]) {
            moveDir -= forwardDir;
        }

        if (keyboardState[keys.moveRight]) {
            moveDir += rightDir;
        }

        if (keyboardState[keys.moveLeft]) {
            moveDir -= rightDir;
        }

        if (keyboardState[keys.moveUp]) {
            moveDir += upDir;
        }

        if (keyboardState[keys.moveDown]) {
            moveDir -= upDir;
        }

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            object.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
    }
}
