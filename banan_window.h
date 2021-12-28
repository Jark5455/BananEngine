#pragma once

#include <tuple>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Banan {
    class BananWindow {

    public:
        BananWindow();

        BananWindow(int w, int h);
        ~BananWindow();

        BananWindow(const BananWindow &) = delete;
        BananWindow &operator=(const BananWindow &) = delete;

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

        bool windowShouldClose();

    private:
        std::tuple<int, int> res = std::make_tuple(0, 0);
        GLFWwindow *window;
    };
}
