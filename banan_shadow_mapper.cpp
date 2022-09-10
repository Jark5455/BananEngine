//
// Created by yashr on 7/2/22.
//

#include "banan_shadow_mapper.h"

#include <stdexcept>

namespace Banan {

    BananShadowMapper::BananShadowMapper(BananDevice &device, id_t id) : bananDevice{device}, id{id} {
        frameBufferDepthFormat = device.findSupportedFormat({VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        createShadowDepthResources();
        createShadowFramebuffers();
    }

    BananShadowMapper::~BananShadowMapper() {
        vkDestroyFramebuffer(bananDevice.device(), frameBuffer, nullptr);
    }

    void BananShadowMapper::createShadowRenderPass(BananDevice &bananDevice) {
        VkAttachmentDescription osAttachments[2] = {};

        // Find a suitable depth format
        VkBool32 validDepthFormat = frameBufferDepthFormat;
        assert(validDepthFormat);

        osAttachments[0].format = VK_FORMAT_R32_SFLOAT;
        osAttachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        osAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        osAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        osAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        osAttachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        osAttachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        osAttachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        osAttachments[1].format = frameBufferDepthFormat;
        osAttachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        osAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        osAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        osAttachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        osAttachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        osAttachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        osAttachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorReference{};
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthReference{};
        depthReference.attachment = 1;
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorReference;
        subpass.pDepthStencilAttachment = &depthReference;

        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 2;
        renderPassCreateInfo.pAttachments = osAttachments;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;

        if (vkCreateRenderPass(bananDevice.device(), &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("unable to create shadow render pass");
        }
    }

    void BananShadowMapper::createShadowDepthResources() {
        bananCubemap = std::make_unique<BananCubemap>(bananDevice, 1024, 1, VK_FORMAT_R32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        bananDevice.transitionImageLayout(bananCubemap->getImageHandle(), VK_FORMAT_R32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 6);
    }

    void BananShadowMapper::createShadowFramebuffers() {

        bananColorImage = std::make_unique<BananImage>(bananDevice, 1024, 1024, 1, VK_FORMAT_R32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        bananDevice.transitionImageLayout(bananColorImage->getImageHandle(), VK_FORMAT_R32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, 1);

        bananDepthImage = std::make_unique<BananImage>(bananDevice, 1024, 1024, 1, frameBufferDepthFormat, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        bananDevice.transitionImageLayout(bananDepthImage->getImageHandle(), frameBufferDepthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, 1);

        VkImageView attachments[2];
        attachments[0] = bananColorImage->descriptorInfo().imageView;
        attachments[1] = bananDepthImage->descriptorInfo().imageView;

        VkFramebufferCreateInfo fbufCreateInfo{};
        fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbufCreateInfo.renderPass = renderPass;
        fbufCreateInfo.attachmentCount = 2;
        fbufCreateInfo.pAttachments = attachments;
        fbufCreateInfo.width = 1024;
        fbufCreateInfo.height = 1024;
        fbufCreateInfo.layers = 1;

        if (vkCreateFramebuffer(bananDevice.device(), &fbufCreateInfo, nullptr, &frameBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow frame buffers");
        }
    }

    VkDescriptorImageInfo BananShadowMapper::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        return bananCubemap->descriptorInfo();
    }

    VkFramebuffer BananShadowMapper::getFramebuffer() {
        return frameBuffer;
    }

    void BananShadowMapper::update(VkCommandBuffer commandBuffer, uint32_t faceindex) {

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = bananColorImage->getImageHandle();
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        //vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        bananDevice.transitionImageLayout(bananColorImage->getImageHandle(), VK_FORMAT_R32_SFLOAT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1, 1);


        VkImageSubresourceRange cubeFaceSubresourceRange = {};
        cubeFaceSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        cubeFaceSubresourceRange.baseMipLevel = 0;
        cubeFaceSubresourceRange.levelCount = 1;
        cubeFaceSubresourceRange.baseArrayLayer = faceindex;
        cubeFaceSubresourceRange.layerCount = 1;

        barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        barrier.image = bananCubemap->getImageHandle();
        barrier.subresourceRange = cubeFaceSubresourceRange;

        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        //vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        bananDevice.transitionImageLayout(bananCubemap->getImageHandle(), VK_FORMAT_R32_SFLOAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 6);

        VkImageCopy copyRegion = {};

        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.srcSubresource.baseArrayLayer = 0;
        copyRegion.srcSubresource.mipLevel = 0;
        copyRegion.srcSubresource.layerCount = 1;
        copyRegion.srcOffset = { 0, 0, 0 };

        copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.dstSubresource.baseArrayLayer = faceindex;
        copyRegion.dstSubresource.mipLevel = 0;
        copyRegion.dstSubresource.layerCount = 1;
        copyRegion.dstOffset = { 0, 0, 0 };

        copyRegion.extent.width = 1024;
        copyRegion.extent.height = 1024;
        copyRegion.extent.depth = 1;

        vkCmdCopyImage(commandBuffer, bananColorImage->getImageHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, bananCubemap->getImageHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        barrier.image = bananColorImage->getImageHandle();
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        barrier.image = bananCubemap->getImageHandle();
        barrier.subresourceRange.baseArrayLayer = faceindex;

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }

    BananShadowMapper::id_t BananShadowMapper::getId() {
        return id;
    }

    VkRenderPass BananShadowMapper::getRenderPass() {
        return renderPass;
    }
}