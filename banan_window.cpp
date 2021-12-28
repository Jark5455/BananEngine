#include "banan_window.h"

#include <iostream>

namespace Banan {

    BananWindow::BananWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        new (this) BananWindow(mode->width, mode->height);
    }

    BananWindow::BananWindow(int w, int h) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(w, h, "BananEngine", nullptr, nullptr);
    }

    bool BananWindow::windowShouldClose() {
        return glfwWindowShouldClose(window);
    }

    BananWindow::~BananWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void BananWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }
}

