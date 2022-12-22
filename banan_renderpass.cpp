//
// Created by yashr on 12/21/22.
//

#include "banan_renderpass.h"

#include <utility>
#include <stdexcept>
#include <algorithm>

namespace Banan {

    BananRenderPass::BananRenderPass(BananDevice &device, std::vector<VkFormat> attachmentFormats, VkExtent2D extent, bool depthImage) : bananDevice{device}, frameBufferFormats{std::move(attachmentFormats)}, extent{extent} {
        if (depthImage)
            frameBufferFormats.push_back(device.findSupportedFormat({VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT));

        createRenderpass();
        createFramebuffer();
    }

    BananRenderPass::~BananRenderPass() {
        vkDestroyFramebuffer(bananDevice.device(), frameBuffer, nullptr);
        vkDestroyRenderPass(bananDevice.device(), renderPass, nullptr);
    }

    void BananRenderPass::createFramebuffer() {
        for (VkFormat &format : frameBufferFormats) {
            frameBufferAttachments.push_back(std::make_shared<BananImage>(bananDevice, extent.width, extent.height, 1, format, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
        }

        VkImageView attachments[frameBufferAttachments.size()];
        for (int i = 0; i < frameBufferAttachments.size(); i++) {
            attachments[i] = frameBufferAttachments[i]->descriptorInfo().imageView;
        }

        VkFramebufferCreateInfo fbufCreateInfo{};
        fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbufCreateInfo.renderPass = renderPass;
        fbufCreateInfo.attachmentCount = frameBufferAttachments.size();
        fbufCreateInfo.pAttachments = attachments;
        fbufCreateInfo.width = extent.width;
        fbufCreateInfo.height = extent.height;
        fbufCreateInfo.layers = 1;

        if (vkCreateFramebuffer(bananDevice.device(), &fbufCreateInfo, nullptr, &frameBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create framebuffer");
        }
    }

    void BananRenderPass::createRenderpass() {
        VkAttachmentDescription osAttachments[frameBufferFormats.size()];

        for (int i = 0; i < frameBufferFormats.size() - 1; i++) {
            osAttachments[i].format = frameBufferFormats[i];
            osAttachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
            osAttachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            osAttachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            osAttachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            osAttachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            osAttachments[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            osAttachments[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        VkFormat depthFormats[] = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM};

        osAttachments[frameBufferFormats.size()].format = frameBufferFormats.back();
        osAttachments[frameBufferFormats.size()].samples = VK_SAMPLE_COUNT_1_BIT;
        osAttachments[frameBufferFormats.size()].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        osAttachments[frameBufferFormats.size()].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        osAttachments[frameBufferFormats.size()].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        osAttachments[frameBufferFormats.size()].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        if (std::find(std::begin(depthFormats), std::end(depthFormats), frameBufferFormats.back()) != std::end(depthFormats)) {
            osAttachments[frameBufferFormats.size()].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            osAttachments[frameBufferFormats.size()].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        } else {
            osAttachments[frameBufferFormats.size()].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            osAttachments[frameBufferFormats.size()].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        std::vector<VkAttachmentReference> colorReferences{};
        std::vector<VkAttachmentReference> depthReferences{};

        for (int i = 0; i < frameBufferFormats.size(); i++) {
            if (osAttachments[i].finalLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                colorReferences.push_back({(uint32_t) i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
            } else if (osAttachments[i].finalLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                depthReferences.push_back({(uint32_t) i, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
            }
        }

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = colorReferences.size();
        subpass.pColorAttachments = colorReferences.data();
        subpass.pDepthStencilAttachment = depthReferences.data();

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = frameBufferFormats.size();
        renderPassCreateInfo.pAttachments = osAttachments;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;

        if (vkCreateRenderPass(bananDevice.device(), &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("unable to create shadow render pass");
        }
    }

    void BananRenderPass::beginRenderPass(VkCommandBuffer commandBuffer, bool flippedViewport) {
        VkClearValue clearValues[frameBufferAttachments.size()];

        for (int i = 0; i < frameBufferAttachments.size() - 1; i++) {
            clearValues[i].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
        }

        VkFormat depthFormats[] = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM};
        if (std::find(std::begin(depthFormats), std::end(depthFormats), frameBufferFormats.back()) != std::end(depthFormats)) {
            clearValues[frameBufferAttachments.size()].depthStencil = { 1.0f, 0 };
        }

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.framebuffer = frameBuffer;
        renderPassBeginInfo.renderArea.extent = extent;
        renderPassBeginInfo.clearValueCount = frameBufferAttachments.size();
        renderPassBeginInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        if (flippedViewport) {
            viewport.x = 0;
            viewport.y = (float) extent.height;
            viewport.width = (float) extent.width;
            viewport.height = (float) -extent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
        } else {
            viewport.x = 0;
            viewport.y = 0;
            viewport.width = (float) extent.width;
            viewport.height = (float) extent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
        }

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.extent.width = extent.width;
        scissor.extent.height = extent.height;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void BananRenderPass::endRenderPass(VkCommandBuffer commandBuffer) {
        vkCmdEndRenderPass(commandBuffer);
    }

    std::vector<std::shared_ptr<BananImage>> BananRenderPass::getFramebufferAttachments() {
        return frameBufferAttachments;
    }

    VkRenderPass BananRenderPass::getRenderPass() {
        return renderPass;
    }
}