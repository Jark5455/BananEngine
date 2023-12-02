#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>

namespace Banan {
    class BananWindow {

    public:
        BananWindow();

        BananWindow(size_t w, size_t h);
        ~BananWindow();

        BananWindow(const BananWindow &) = delete;
        BananWindow &operator=(const BananWindow &) = delete;

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
        VkExtent2D getExtent();
        SDL_Window *getSDLWindow() const;

    private:
        size_t width;
        size_t height;

        SDL_Window *window;
    };
}
