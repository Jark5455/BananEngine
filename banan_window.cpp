#include "banan_window.h"

#include <iostream>

namespace Banan {
    BananWindow::BananWindow() {
        SDL_GetWindowSize(window, &width, &height);
        new (this) BananWindow(width, height);
    }

    BananWindow::BananWindow(int w, int h) : width{w}, height{h} {
        SDL_Init(SDL_INIT_EVERYTHING);
        window = SDL_CreateWindow("Banan",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, w, h,SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    }

    BananWindow::~BananWindow() {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void BananWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        if (SDL_Vulkan_CreateSurface(window, instance, surface) != SDL_TRUE) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    VkExtent2D BananWindow::getExtent() {
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    SDL_Window *BananWindow::getSDLWindow() const {
        return window;
    }
}

