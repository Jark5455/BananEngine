//
// Created by yashr on 12/7/21.
//

#pragma once

#include "banan_window.h"

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace Banan {
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices {
        size_t graphicsFamily;
        size_t presentFamily;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

    class BananDevice {
    public:
        const bool enableValidationLayers = true;

        BananDevice(BananWindow &window);
        ~BananDevice();

        BananDevice(BananDevice &&) = delete;
        BananDevice &operator=(BananDevice &&) = delete;

        VkCommandPool getCommandPool() { return commandPool; }
        VkDevice device() { return device_; }
        VkSurfaceKHR surface() { return surface_; }
        VkQueue graphicsQueue() { return graphicsQueue_; }
        VkQueue presentQueue() { return presentQueue_; }
        VkPhysicalDeviceProperties physicalDeviceProperties() { return properties; }
        VkSampleCountFlagBits getMsaaSampleCount() { return msaaSamples; }

        SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
        bool checkMemoryType(VkMemoryPropertyFlags properties);
        size_t findMemoryType(size_t typeFilter, VkMemoryPropertyFlags properties);
        QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
        VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        VkSampleCountFlagBits getMaxUsableSampleCount();

        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, size_t mipLevels, size_t layerCount);
        void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, size_t mipLevels, size_t layerCount);
        void copyBufferToImage(VkBuffer buffer, VkImage image, size_t width, size_t height, size_t layerCount);
        void generateMipMaps(VkImage image, size_t width, size_t height, size_t mipLevels);

        void createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);

    private:
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();

        // helper functions
        bool isDeviceSuitable(VkPhysicalDevice device);
        std::vector<const char *> getRequiredExtensions();
        bool checkValidationLayerSupport();
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        void hasGflwRequiredInstanceExtensions();
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceProperties properties;
        BananWindow &bananWindow;
        VkCommandPool commandPool;

        VkDevice device_;
        VkSurfaceKHR surface_;
        VkQueue graphicsQueue_;
        VkQueue presentQueue_;

        VkSampleCountFlagBits msaaSamples;

        const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                            VK_KHR_MAINTENANCE1_EXTENSION_NAME,
                                                            VK_KHR_MAINTENANCE2_EXTENSION_NAME,
                                                            VK_KHR_MAINTENANCE3_EXTENSION_NAME,
                                                            VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
                                                            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                                                            VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
                                                            VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
                                                            VK_KHR_MULTIVIEW_EXTENSION_NAME,
                                                            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME};
    };
}
