//
// Created by yashr on 1/8/22.
//

#pragma once

#include <banan_game_object.h>
#include <banan_window.h>

namespace Banan {
    class KeyboardMovementController {
        public:

        enum KeyMappings {
            moveLeft = SDLK_a,
            moveRight = SDLK_d,
            moveForward = SDLK_w,
            moveBackward = SDLK_s,

            moveUp = SDLK_SPACE,
            moveDown = SDLK_LCTRL,

            lookLeft = SDLK_LEFT,
            lookRight = SDLK_RIGHT,
            lookUp = SDLK_UP,
            lookDown = SDLK_DOWN,
        };

        void moveInPlaneXZ(SDL_Window *window, float dt, BananGameObject &object);

        float moveSpeed{3.f};
        float lookSpeed{1.5f};
    };
}