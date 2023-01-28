//
// Created by yashr on 1/8/22.
//

#include "KeyboardMovementController.h"

#include <limits>
#include <iostream>

namespace Banan {
    void KeyboardMovementController::moveInPlaneXZ(SDL_Window *window, float dt, Banan::BananGameObject &object) {
        glm::vec3 rotate{0.f};
        glm::vec3 moveDir{0.f};

        SDL_Event event;
        SDL_PollEvent(&event);

        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case KeyMappings::lookRight:
                    rotate.y += 1.f;
                    break;
                case KeyMappings::lookLeft:
                    rotate.y -= 1.f;
                    break;
                case KeyMappings::lookDown:
                    rotate.x += 1.f;
                    break;
                case KeyMappings::lookUp:
                    rotate.x -= 1.f;
                    break;
            }
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

        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case KeyMappings::moveForward:
                    moveDir += forwardDir;
                    break;
                case KeyMappings::moveBackward:
                    moveDir -= forwardDir;
                    break;
                case KeyMappings::moveRight:
                    moveDir += rightDir;
                    break;
                case KeyMappings::moveLeft:
                    moveDir -= rightDir;
                    break;
                case KeyMappings::moveUp:
                    moveDir += upDir;
                    break;
                case KeyMappings::moveDown:
                    moveDir -= upDir;
                    break;
            }
        }

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            object.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
    }
}
