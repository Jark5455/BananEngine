//
// Created by yashr on 12/30/21.
//

#include "banan_swap_chain.h"

#include <array>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>

namespace Banan {
    BananSwapChain::BananSwapChain(BananDevice &deviceRef, VkExtent2D extent, std::shared_ptr<BananSwapChain> previous) : device{deviceRef}, windowExtent{extent}, oldSwapChain{std::move(previous)} {
        createSwapChain();
        createImageViews();
        createGBufferResources();
        createResolveResources();
        createGeometryRenderPass();
        createGeometryFramebuffer();
        createResolveRenderpasses();
        createResolveFramebuffers();
        createSyncObjects();
    }

    BananSwapChain::~BananSwapChain() {
        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device.device(), imageView, nullptr);
        }
        swapChainImageViews.clear();

        if (swapChain != nullptr) {
            vkDestroySwapchainKHR(device.device(), swapChain, nullptr);
            swapChain = nullptr;
        }

        vkDestroyFramebuffer(device.device(), geometryFramebuffer, nullptr);
        vkDestroyRenderPass(device.device(), geometryRenderpass, nullptr);

        vkDestroyFramebuffer(device.device(), edgeDetectionFramebuffer, nullptr);
        vkDestroyRenderPass(device.device(), edgeDetectionRenderPass, nullptr);

        vkDestroyFramebuffer(device.device(), blendFramebuffer, nullptr);
        vkDestroyRenderPass(device.device(), blendWeightRenderPass, nullptr);

        for (auto framebuffer : resolveFramebuffers) {
            vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
        }

        vkDestroyRenderPass(device.device(), resolveRenderPass, nullptr);

        // cleanup synchronization objects
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device.device(), renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device.device(), imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device.device(), inFlightFences[i], nullptr);
        }
    }

    VkResult BananSwapChain::acquireNextImage(size_t *imageIndex) {
        vkWaitForFences(device.device(), 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
        VkResult result = vkAcquireNextImageKHR(device.device(), swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, reinterpret_cast<uint32_t *>(const_cast<size_t *>(imageIndex)));
        return result;
    }

    VkResult BananSwapChain::submitCommandBuffers(const VkCommandBuffer *buffers, const size_t *imageIndex) {
        if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(device.device(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
        }
        imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(device.device(), 1, &inFlightFences[currentFrame]);
        if (vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = reinterpret_cast<uint32_t *>(const_cast<size_t *>(imageIndex));

        auto result = vkQueuePresentKHR(device.presentQueue(), &presentInfo);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return result;
    }

    void BananSwapChain::createSwapChain() {
        SwapChainSupportDetails swapChainSupport = device.getSwapChainSupport();

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = device.surface();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = device.findPhysicalQueueFamilies();
        uint32_t queueFamilyIndices[] = {static_cast<uint32_t>(indices.graphicsFamily), static_cast<uint32_t>(indices.presentFamily)};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;      // Optional
            createInfo.pQueueFamilyIndices = nullptr;  // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain;

        if (vkCreateSwapchainKHR(device.device(), &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        // we only specified a minimum number of images in the swap chain, so the implementation is
        // allowed to create a swap chain with more. That's why we'll first query the final number of
        // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
        // retrieve the handles.
        vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void BananSwapChain::createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = swapChainImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = swapChainImageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device.device(), &viewInfo, nullptr, &swapChainImageViews[i]) !=
                VK_SUCCESS) {
                throw std::runtime_error("failed to create texture image view!");
            }
        }
    }

    void BananSwapChain::createGeometryRenderPass() {

        std::vector<VkAttachmentDescription> attachments{4};

        // depth attachment
        attachments[0].format = swapChainDepthFormat;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // gbuffer normals attachment
        attachments[1].format = VK_FORMAT_R16G16B16A16_SFLOAT;
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // gbuffer albedo attachment
        attachments[2].format = VK_FORMAT_R8G8B8A8_UNORM;
        attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // composite color attachment
        attachments[3].format = VK_FORMAT_R8G8B8A8_UNORM;
        attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[3].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference depthReference = {0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        std::vector<VkAttachmentReference> gbufferReferences{2};
        gbufferReferences[0].attachment = 1;
        gbufferReferences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        gbufferReferences[1].attachment = 2;
        gbufferReferences[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription gBufferSubpass{};
        gBufferSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        gBufferSubpass.colorAttachmentCount = static_cast<uint32_t>(gbufferReferences.size());
        gBufferSubpass.pColorAttachments = gbufferReferences.data();
        gBufferSubpass.pDepthStencilAttachment = &depthReference;

        VkAttachmentReference compositeReferences{3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        std::vector<VkAttachmentReference> inputAttachments{3};
        inputAttachments[0] = depthReference;
        inputAttachments[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        inputAttachments[1].attachment = 1;
        inputAttachments[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        inputAttachments[2].attachment = 2;
        inputAttachments[2].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkSubpassDescription compositeSubpass{};
        compositeSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        compositeSubpass.colorAttachmentCount = 1;
        compositeSubpass.pColorAttachments = &compositeReferences;
        compositeSubpass.pDepthStencilAttachment = nullptr;
        compositeSubpass.inputAttachmentCount = static_cast<uint32_t>(inputAttachments.size());
        compositeSubpass.pInputAttachments = inputAttachments.data();

        VkSubpassDescription forwardSubpass{};
        forwardSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        forwardSubpass.colorAttachmentCount = 1;
        forwardSubpass.pColorAttachments = &compositeReferences;
        forwardSubpass.pDepthStencilAttachment = &depthReference;

        std::vector<VkSubpassDependency> dependencies{5};

        // This dependency transitions the input attachment from color attachment to shader read
        dependencies[0].srcSubpass = 0;
        dependencies[0].dstSubpass = 1;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // This dependency transitions the input attachment from depth attachment to shader read
        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = 1;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // This dependency transitions the depth attachment back to write and read
        dependencies[2].srcSubpass = 1;
        dependencies[2].dstSubpass = 2;
        dependencies[2].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[2].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[2].srcAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
        dependencies[2].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[2].dependencyFlags  = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[3].srcSubpass = 2;
        dependencies[3].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[3].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[3].dstStageMask =  VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        dependencies[3].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[3].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[3].dependencyFlags  = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[4].srcSubpass = 2;
        dependencies[4].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[4].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[4].dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        dependencies[4].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[4].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[4].dependencyFlags  = VK_DEPENDENCY_BY_REGION_BIT;

        std::vector<VkSubpassDescription> subpasses{3};
        subpasses[0] = gBufferSubpass;
        subpasses[1] = compositeSubpass;
        subpasses[2] = forwardSubpass;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
        renderPassInfo.pSubpasses = subpasses.data();
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &geometryRenderpass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void BananSwapChain::createGeometryFramebuffer() {
        std::array<VkImageView,4> attachments = {gBufferAttachments[0]->getImageView(), gBufferAttachments[1]->getImageView(), gBufferAttachments[2]->getImageView(), geometryImage->getImageView()};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = geometryRenderpass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &geometryFramebuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

    void BananSwapChain::createResolveRenderpasses() {

        // edge detection pass
        {
            VkAttachmentDescription edgeAttachment{};

            // techically this could be R8G8 but for some reason its slower on nvidia, will test on amd later
            edgeAttachment.format = VK_FORMAT_R8G8_UNORM;
            edgeAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            edgeAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            edgeAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            edgeAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            edgeAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            edgeAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            edgeAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkAttachmentReference edgeAttachmentReference{};
            edgeAttachmentReference.attachment = 0;
            edgeAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription edgeDetectionSubpass{};
            edgeDetectionSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            edgeDetectionSubpass.colorAttachmentCount = 1;
            edgeDetectionSubpass.pColorAttachments = &edgeAttachmentReference;

            std::vector<VkSubpassDependency> dependencies{1};

            // shader read only
            dependencies[0].srcSubpass = 0;
            dependencies[0].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            VkRenderPassCreateInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments = &edgeAttachment;
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &edgeDetectionSubpass;
            renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
            renderPassInfo.pDependencies = dependencies.data();

            if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &edgeDetectionRenderPass) != VK_SUCCESS) {
                throw std::runtime_error("Unable to create edge detection renderpass for SMAA");
            }
        }

        // blend weight pass
        {
            VkAttachmentDescription blendAttachment{};

            // techically this could be R8G8 but for some reason its slower on nvidia, will test on amd later
            blendAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
            blendAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            blendAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            blendAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            blendAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            blendAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            blendAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            blendAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            VkAttachmentReference blendAttachmentReference{};
            blendAttachmentReference.attachment = 0;
            blendAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription blendAttachmentSubpass{};
            blendAttachmentSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            blendAttachmentSubpass.colorAttachmentCount = 1;
            blendAttachmentSubpass.pColorAttachments = &blendAttachmentReference;

            std::vector<VkSubpassDependency> dependencies{1};

            // shader read only
            dependencies[0].srcSubpass = 0;
            dependencies[0].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            VkRenderPassCreateInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments = &blendAttachment;
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &blendAttachmentSubpass;
            renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
            renderPassInfo.pDependencies = dependencies.data();

            if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &blendWeightRenderPass) != VK_SUCCESS) {
                throw std::runtime_error("Unable to create edge detection renderpass for SMAA");
            }
        }

        // resolve pass
        {
            VkAttachmentDescription resolveAttachment{};

            // techically this could be R8G8 but for some reason its slower on nvidia, will test on amd later
            resolveAttachment.format = swapChainImageFormat;
            resolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            resolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            resolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            resolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            resolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            resolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            resolveAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference resolveAttachmentReference{};
            resolveAttachmentReference.attachment = 0;
            resolveAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription resolveAttachmentSubpass{};
            resolveAttachmentSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            resolveAttachmentSubpass.colorAttachmentCount = 1;
            resolveAttachmentSubpass.pColorAttachments = &resolveAttachmentReference;

            std::vector<VkSubpassDependency> dependencies{2};

            // convert to writeable
            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].srcAccessMask = 0;
            dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            // convert to presentable
            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = 0;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            VkRenderPassCreateInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments = &resolveAttachment;
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &resolveAttachmentSubpass;
            renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
            renderPassInfo.pDependencies = dependencies.data();

            if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &resolveRenderPass) != VK_SUCCESS) {
                throw std::runtime_error("Unable to create edge detection renderpass for SMAA");
            }
        }
    }

    void BananSwapChain::createResolveFramebuffers() {

        {
            VkImageView edgeView = edgeImage->getImageView();

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = edgeDetectionRenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &edgeView;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &edgeDetectionFramebuffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to create edge framebuffer!");
            }
        }

        {
            VkImageView blendView = blendImage->getImageView();

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = blendWeightRenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = &blendView;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &blendFramebuffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to create blend framebuffer!");
            }
        }

        {
            resolveFramebuffers.resize(imageCount());
            for (size_t i = 0; i < imageCount(); i++) {
                VkImageView colorView = swapChainImageViews[i];

                VkFramebufferCreateInfo framebufferInfo = {};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = resolveRenderPass;
                framebufferInfo.attachmentCount = 1;
                framebufferInfo.pAttachments = &colorView;
                framebufferInfo.width = swapChainExtent.width;
                framebufferInfo.height = swapChainExtent.height;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &resolveFramebuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create blend framebuffer!");
                }
            }
        }
    }

    void BananSwapChain::createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) !=
                VK_SUCCESS ||
                vkCreateSemaphore(device.device(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) !=
                VK_SUCCESS ||
                vkCreateFence(device.device(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void BananSwapChain::createGBufferResources() {
        swapChainDepthFormat =  findDepthFormat();
        VkMemoryPropertyFlags memoryPropertyFlags = device.checkMemoryType(VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        gBufferAttachments.push_back(std::make_shared<BananImage>(device, static_cast<size_t>(swapChainExtent.width), static_cast<size_t>(swapChainExtent.height), 1, 1, swapChainDepthFormat, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, memoryPropertyFlags));
        gBufferAttachments.push_back(std::make_shared<BananImage>(device, static_cast<size_t>(swapChainExtent.width), static_cast<size_t>(swapChainExtent.height), 1, 1, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, memoryPropertyFlags));
        gBufferAttachments.push_back(std::make_shared<BananImage>(device, static_cast<size_t>(swapChainExtent.width), static_cast<size_t>(swapChainExtent.height), 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, memoryPropertyFlags));
    }

    void BananSwapChain::createResolveResources() {
        geometryImage = std::make_shared<BananImage>(device, static_cast<size_t>(swapChainExtent.width), static_cast<size_t>(swapChainExtent.height), 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        edgeImage = std::make_shared<BananImage>(device, static_cast<size_t>(swapChainExtent.width), static_cast<size_t>(swapChainExtent.height), 1, 1, VK_FORMAT_R8G8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        blendImage = std::make_shared<BananImage>(device, static_cast<size_t>(swapChainExtent.width), static_cast<size_t>(swapChainExtent.height), 1, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    VkSurfaceFormatKHR BananSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR BananSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for (const auto &availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                std::cout << "Present mode: Mailbox" << std::endl;
                return availablePresentMode;
            }
        }

        std::cout << "Present mode: V-Sync" << std::endl;
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D BananSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            VkExtent2D actualExtent = windowExtent;
            actualExtent.width = std::max(capabilities.minImageExtent.width,std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height,std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    VkFormat BananSwapChain::findDepthFormat() {
        return device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},VK_IMAGE_TILING_OPTIMAL,VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    bool BananSwapChain::compareSwapFormats(const BananSwapChain &otherSwapChain) const {
        return otherSwapChain.swapChainDepthFormat == swapChainDepthFormat && otherSwapChain.swapChainImageFormat == swapChainImageFormat;
    }
}
