//
// Created by yashr on 12/30/21.
//

#pragma once

#include "banan_device.h"
#include "banan_image.h"

// vulkan headers
#include <volk.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace Banan {
    class BananSwapChain {
    public:

        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
        BananSwapChain(BananDevice &deviceRef, VkExtent2D extent, std::shared_ptr<BananSwapChain> previous);
        ~BananSwapChain();

        BananSwapChain(const BananSwapChain &) = delete;
        BananSwapChain &operator=(const BananSwapChain &) = delete;

        VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
        VkRenderPass getRenderPass() { return renderPass; }
        VkImageView getImageView(int index) { return swapChainImageViews[index]; }
        size_t imageCount() { return swapChainImages.size(); }
        VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
        VkExtent2D getSwapChainExtent() { return swapChainExtent; }
        uint32_t width() { return swapChainExtent.width; }
        uint32_t height() { return swapChainExtent.height; }
        std::vector<std::shared_ptr<BananImage>> gbuffer() { return gBufferAttachments; }

        float extentAspectRatio() {
            return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }

        VkFormat findDepthFormat();

        VkResult acquireNextImage(uint32_t *imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer *buffers, const uint32_t *imageIndex);

        bool compareSwapFormats(const BananSwapChain &otherSwapChain) const;

    private:
        void createSwapChain();
        void createImageViews();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();
        void createGBufferResources();

        // Helper functions
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

        VkFormat swapChainImageFormat;
        VkFormat swapChainDepthFormat;
        VkExtent2D swapChainExtent;

        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkRenderPass renderPass;

        std::vector<std::shared_ptr<BananImage>> gBufferAttachments;

        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        BananDevice &device;
        VkExtent2D windowExtent;

        VkSwapchainKHR swapChain;
        std::shared_ptr<BananSwapChain> oldSwapChain;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        size_t currentFrame = 0;
    };
}
