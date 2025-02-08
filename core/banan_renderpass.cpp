//
// Created by yashr on 12/21/22.
//

#include "banan_renderpass.h"

#include <utility>
#include <stdexcept>
#include <algorithm>

namespace Banan {

    BananRenderPass::BananRenderPass(BananDevice &device, std::vector<VkFormat> attachmentFormats, VkExtent2D extent, bool depthImage) : frameBufferFormats{std::move(attachmentFormats)}, bananDevice{device}, extent{extent} {
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

        VkFormat depthFormats[] = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM};
        for (size_t i = 0 ; i < frameBufferFormats.size() - 1; i++) {
            frameBufferAttachments.push_back(std::make_shared<BananImage>(bananDevice, extent.width, extent.height, 1, frameBufferFormats[i], VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
            bananDevice.transitionImageLayout(frameBufferAttachments.back()->getImageHandle(), frameBufferFormats[i], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, 1);
        }

        if (std::find(std::begin(depthFormats), std::end(depthFormats), frameBufferFormats.back()) != std::end(depthFormats)) {
            frameBufferAttachments.push_back(std::make_shared<BananImage>(bananDevice, extent.width, extent.height, 1, frameBufferFormats[frameBufferFormats.size() - 1], VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
            bananDevice.transitionImageLayout(frameBufferAttachments.back()->getImageHandle(), frameBufferFormats.back(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, 1);
        } else {
            frameBufferAttachments.push_back(std::make_shared<BananImage>(bananDevice, extent.width, extent.height, 1, frameBufferFormats[frameBufferFormats.size() - 1], VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
            bananDevice.transitionImageLayout(frameBufferAttachments.back()->getImageHandle(), frameBufferFormats.back(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, 1);
        }

        std::vector<VkImageView> attachments{frameBufferAttachments.size()};
        for (size_t i = 0; i < attachments.size(); i++) {
            attachments[i] = frameBufferAttachments[i]->descriptorInfo().imageView;
        }

        VkFramebufferCreateInfo fbufCreateInfo{};
        fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbufCreateInfo.renderPass = renderPass;
        fbufCreateInfo.attachmentCount = attachments.size();
        fbufCreateInfo.pAttachments = attachments.data();
        fbufCreateInfo.width = extent.width;
        fbufCreateInfo.height = extent.height;
        fbufCreateInfo.layers = 1;

        if (vkCreateFramebuffer(bananDevice.device(), &fbufCreateInfo, nullptr, &frameBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create framebuffer");
        }
    }

    void BananRenderPass::createRenderpass() {
        VkFormat depthFormats[] = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM};
        std::vector<VkAttachmentDescription> osAttachments{frameBufferFormats.size()};

        for (size_t i = 0; i < osAttachments.size(); i++) {
            osAttachments[i].format = frameBufferFormats[i];
            osAttachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
            osAttachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            osAttachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            osAttachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            osAttachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            osAttachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            osAttachments[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            osAttachments[i].flags = 0;
        }

        std::vector<VkAttachmentReference> colorReferences{};
        std::vector<VkAttachmentReference> depthReferences{};

        for (size_t i = 0; i < frameBufferFormats.size() - 1; i++) {
            VkAttachmentReference colorReference{};
            colorReference.attachment = i;
            colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorReferences.push_back(colorReference);
        }

        if (std::find(std::begin(depthFormats), std::end(depthFormats), frameBufferFormats.back()) != std::end(depthFormats)) {
            VkAttachmentReference depthReference{};
            depthReference.attachment = osAttachments.size() - 1;
            depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthReferences.push_back(depthReference);
        } else {
            VkAttachmentReference colorReference{};
            colorReference.attachment = osAttachments.size() - 1;
            colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorReferences.push_back(colorReference);
        }

        std::vector<VkSubpassDependency> subpassDepends{2};

        subpassDepends[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDepends[0].dstSubpass = 0;
        subpassDepends[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDepends[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDepends[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpassDepends[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDepends[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        subpassDepends[1].srcSubpass = 0;
        subpassDepends[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        subpassDepends[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDepends[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDepends[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDepends[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpassDepends[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = colorReferences.size();
        subpass.pColorAttachments = colorReferences.data();
        subpass.pDepthStencilAttachment = depthReferences.data();

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = osAttachments.size();
        renderPassCreateInfo.pAttachments = osAttachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = 2;
        renderPassCreateInfo.pDependencies = subpassDepends.data();

        if (vkCreateRenderPass(bananDevice.device(), &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("unable to create shadow render pass");
        }
    }

    void BananRenderPass::beginRenderPass(VkCommandBuffer commandBuffer, bool flippedViewport) {
        std::vector<VkClearValue> clearValues{frameBufferAttachments.size()};

        for (size_t i = 0; i < clearValues.size() - 1; i++) {
            clearValues[i].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
        }

        VkFormat depthFormats[] = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM};
        if (std::find(std::begin(depthFormats), std::end(depthFormats), frameBufferFormats.back()) != std::end(depthFormats)) {
            clearValues[clearValues.size() - 1].depthStencil = { 1.0f, 0 };
        }

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.framebuffer = frameBuffer;
        renderPassBeginInfo.renderArea.extent = extent;
        renderPassBeginInfo.clearValueCount = clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();

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