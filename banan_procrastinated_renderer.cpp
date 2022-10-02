//
// Created by yashr on 10/2/22.
//

#include "banan_procrastinated_renderer.h"

#include <stdexcept>
#include <cassert>

namespace Banan {

    BananProcrastinatedRenderer::BananProcrastinatedRenderer(BananWindow &window, BananDevice &device) : bananDevice{device}, bananWindow(window) {
        recreateSwapChain();
        createCommandBuffers();

        depthFormat = bananSwapChain->findDepthFormat();

        createOffscreenRenderpass();
        createOffscreenImages();
        createOffscreenFramebuffers();
    }

    BananProcrastinatedRenderer::~BananProcrastinatedRenderer() {
        vkDestroyFramebuffer(bananDevice.device(), procrastinatedFramebuffer, nullptr);
        vkDestroyRenderPass(bananDevice.device(), renderPass, nullptr);
    }

    void BananProcrastinatedRenderer::createOffscreenFramebuffers() {
        std::vector<VkImageView> attachments{};
        attachments[0] = positionImage->descriptorInfo().imageView;
        attachments[1] = colorImage->descriptorInfo().imageView;
        attachments[2] = normalImage->descriptorInfo().imageView;
        attachments[3] = depthImage->descriptorInfo().imageView;

        VkFramebufferCreateInfo fbufCreateInfo = {};
        fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbufCreateInfo.renderPass = renderPass;
        fbufCreateInfo.pAttachments = attachments.data();
        fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        fbufCreateInfo.width = bananSwapChain->getSwapChainExtent().width;
        fbufCreateInfo.height = bananSwapChain->getSwapChainExtent().height;
        fbufCreateInfo.layers = 1;

        if (vkCreateFramebuffer(bananDevice.device(), &fbufCreateInfo, nullptr, &procrastinatedFramebuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create prcrastinated framebuffer");
        }
    }

    void BananProcrastinatedRenderer::createOffscreenImages() {
        positionImage = std::make_unique<BananImage>(bananDevice, bananWindow.getExtent().width, bananWindow.getExtent().height, 1, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        colorImage = std::make_unique<BananImage>(bananDevice, bananWindow.getExtent().width, bananWindow.getExtent().height, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        normalImage = std::make_unique<BananImage>(bananDevice, bananWindow.getExtent().width, bananWindow.getExtent().height, 1, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        depthImage = std::make_unique<BananImage>(bananDevice, bananWindow.getExtent().width, bananWindow.getExtent().height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    void BananProcrastinatedRenderer::createOffscreenRenderpass() {
        std::vector<VkAttachmentDescription> attachments{};

        //position
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachments[0].format = VK_FORMAT_R16G16B16A16_SFLOAT;

        //color
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachments[1].format = VK_FORMAT_R8G8B8A8_UNORM;

        //normal
        attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[2].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachments[2].format = VK_FORMAT_R16G16B16A16_SFLOAT;

        //depth
        attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments[3].format = depthFormat;

        // attachment refs
        std::vector<VkAttachmentReference> colorReferences;
        colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
        colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
        colorReferences.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

        VkAttachmentReference depthReference = {};
        depthReference.attachment = 3;
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.pColorAttachments = colorReferences.data();
        subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
        subpass.pDepthStencilAttachment = &depthReference;

        // layout transitions
        std::vector<VkSubpassDependency> dependencies;
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        //renderpass
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(bananDevice.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("unable to create procrastinated rendering render pass");
        }
    }

    void BananProcrastinatedRenderer::recreateSwapChain() {
        auto extent = bananWindow.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = bananWindow.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(bananDevice.device());
        if (bananSwapChain == nullptr) {
            bananSwapChain = std::make_unique<BananSwapChain>(bananDevice, extent, nullptr);
        } else {
            std::shared_ptr<BananSwapChain> oldSwapChain = std::move(bananSwapChain);
            bananSwapChain = std::make_unique<BananSwapChain>(bananDevice, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*bananSwapChain)) {
                throw std::runtime_error("Swap chain image or depth format has changed");
            }

            assert(bananSwapChain->imageCount() == oldSwapChain->imageCount() && "Swap chain image count has changed!");
        }
    }

    void BananProcrastinatedRenderer::createCommandBuffers() {

    }
}