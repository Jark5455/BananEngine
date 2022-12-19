//
// Created by yashr on 1/8/22.
//

#pragma once

#include <banan_game_object.h>
#include <banan_window.h>

namespace Banan {
    class KeyboardMovementController {
        public:
        struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;

            int moveUp = GLFW_KEY_SPACE;
            int moveDown = GLFW_KEY_LEFT_CONTROL;

            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
        };

        void moveInPlaneXZ(GLFWwindow *window, float dt, BananGameObject &object);

        KeyMappings keys{};
        float moveSpeed{3.f};
        float lookSpeed{1.5f};
    };
}