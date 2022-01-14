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

        bool wasWindowResized() { return framebufferResized; }
        void resetWindowResizedFlag() { framebufferResized = false; }
        GLFWwindow* getGLFWwindow() const;

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
        VkExtent2D getExtent();
        bool windowShouldClose();

    private:
        static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

        int width;
        int height;

        bool framebufferResized = false;

        GLFWwindow *window;
    };
}
