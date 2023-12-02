#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>

namespace Banan {
    class BananWindow {

    public:
        BananWindow();

        BananWindow(int w, int h);
        ~BananWindow();

        BananWindow(const BananWindow &) = delete;
        BananWindow &operator=(const BananWindow &) = delete;

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
        VkExtent2D getExtent();
        SDL_Window *getSDLWindow() const;

    private:
        int width;
        int height;

        SDL_Window *window;
    };
}
