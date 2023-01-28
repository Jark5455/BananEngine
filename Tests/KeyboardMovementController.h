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
            int moveLeft = SDL_SCANCODE_A;
            int moveRight = SDL_SCANCODE_D;
            int moveForward = SDL_SCANCODE_W;
            int moveBackward = SDL_SCANCODE_S;

            int moveUp = SDL_SCANCODE_SPACE;
            int moveDown = SDL_SCANCODE_LCTRL;

            int lookLeft = SDL_SCANCODE_LEFT;
            int lookRight = SDL_SCANCODE_RIGHT;
            int lookUp = SDL_SCANCODE_UP;
            int lookDown = SDL_SCANCODE_DOWN;
        };

        void moveInPlaneXZ(float dt, BananGameObject &object);

        float moveSpeed{3.f};
        float lookSpeed{1.5f};
        KeyMappings keys{};
    };
}