#include "banan_window.h"

#include <iostream>

namespace Banan {

    BananWindow::BananWindow() {
        const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        new (this) BananWindow(mode->width, mode->height);
    }

    BananWindow::BananWindow(int w, int h) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(w, h, "BananEngine", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
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

    VkExtent2D BananWindow::getExtent() {
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    void BananWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
        auto bananWindow = reinterpret_cast<BananWindow *>(glfwGetWindowUserPointer(window));
        bananWindow->framebufferResized = true;
        bananWindow->width = width;
        bananWindow->height = height;
    }

    GLFWwindow *BananWindow::getGLFWwindow() const {
        return window;
    }
}

