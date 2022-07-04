//
// Created by yashr on 7/2/22.
//

#include "banan_shadow_mapper.h"

#include <stdexcept>

namespace Banan {

    BananShadowMapper::BananShadowMapper(BananDevice &device) : bananDevice{device} {
        frameBufferDepthFormat = VK_FORMAT_R32_SFLOAT;

        createShadowDepthResources();
        createShadowRenderPass();
        createShadowFramebuffers();
    }

    BananShadowMapper::~BananShadowMapper() {
        vkDestroyImageView(bananDevice.device(), shadowDepthCubemapImageView, nullptr);
        vkDestroyImage(bananDevice.device(), shadowDepthCubemapImage, nullptr);
        vkDestroySampler(bananDevice.device(), shadowDepthCubemapImageSampler, nullptr);
        vkFreeMemory(bananDevice.device(), shadowDepthCubemapImageMemory, nullptr);

        vkDestroyImageView(bananDevice.device(), shadowDepthImageView, nullptr);
        vkDestroyImage(bananDevice.device(), shadowDepthImage, nullptr);
        vkFreeMemory(bananDevice.device(), shadowDepthImageMemory, nullptr);

        vkDestroyImageView(bananDevice.device(), shadowColorImageView, nullptr);
        vkDestroyImage(bananDevice.device(), shadowColorImage, nullptr);
        vkFreeMemory(bananDevice.device(), shadowColorImageMemory, nullptr);

        vkDestroyFramebuffer(bananDevice.device(), frameBuffer, nullptr);
        vkDestroyRenderPass(bananDevice.device(), renderPass, nullptr);
    }

    void BananShadowMapper::createShadowRenderPass() {
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
        VkFormat format = VK_FORMAT_R32_SFLOAT;

        // Cube map image description
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = format;
        imageCreateInfo.extent = {1024, 1024, 1};
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 6;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        bananDevice.createImageWithInfo(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shadowDepthCubemapImage,
                                        shadowDepthCubemapImageMemory);
        bananDevice.transitionImageLayout(shadowDepthCubemapImage, format, VK_IMAGE_LAYOUT_UNDEFINED,
                                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 6);

        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.maxAnisotropy = 1.0f;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 1.0f;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        if (vkCreateSampler(bananDevice.device(), &samplerCreateInfo, nullptr, &shadowDepthCubemapImageSampler) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow depth cubemap sampler");
        }

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = shadowDepthCubemapImage;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        imageViewCreateInfo.format = format;
        imageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_R};
        imageViewCreateInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        imageViewCreateInfo.subresourceRange.layerCount = 6;

        if (vkCreateImageView(bananDevice.device(), &imageViewCreateInfo, nullptr, &shadowDepthCubemapImageView) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow depth cubemap image view");
        }
    }

    void BananShadowMapper::createShadowFramebuffers() {
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = VK_FORMAT_R32_SFLOAT;
        imageCreateInfo.extent.width = 1024;
        imageCreateInfo.extent.height = 1024;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        // Image of the framebuffer is blit source
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        bananDevice.createImageWithInfo(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shadowColorImage,
                                        shadowColorImageMemory);
        bananDevice.transitionImageLayout(shadowColorImage, VK_FORMAT_R32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED,
                                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, 1);

        VkImageViewCreateInfo colorImageViewCreateInfo{};
        colorImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        colorImageViewCreateInfo.image = shadowColorImage;
        colorImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        colorImageViewCreateInfo.format = VK_FORMAT_R32_SFLOAT;
        colorImageViewCreateInfo.flags = 0;
        colorImageViewCreateInfo.subresourceRange = {};
        colorImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        colorImageViewCreateInfo.subresourceRange.levelCount = 1;
        colorImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        colorImageViewCreateInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(bananDevice.device(), &colorImageViewCreateInfo, nullptr, &shadowColorImageView) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow color image view");
        }

        VkImageCreateInfo depthImageCreateInfo{};
        depthImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        depthImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        depthImageCreateInfo.format = frameBufferDepthFormat;
        depthImageCreateInfo.extent.width = 1024;
        depthImageCreateInfo.extent.height = 1024;
        depthImageCreateInfo.extent.depth = 1;
        depthImageCreateInfo.mipLevels = 1;
        depthImageCreateInfo.arrayLayers = 1;
        depthImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        depthImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        // Image of the framebuffer is blit source
        depthImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        depthImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        bananDevice.createImageWithInfo(depthImageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shadowDepthImage,
                                        shadowDepthImageMemory);
        bananDevice.transitionImageLayout(shadowDepthImage, frameBufferDepthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1);

        VkImageViewCreateInfo depthStencilView{};
        depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depthStencilView.format = frameBufferDepthFormat;
        depthStencilView.flags = 0;
        depthStencilView.subresourceRange = {};
        depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        depthStencilView.subresourceRange.baseMipLevel = 0;
        depthStencilView.subresourceRange.levelCount = 1;
        depthStencilView.subresourceRange.baseArrayLayer = 0;
        depthStencilView.subresourceRange.layerCount = 1;
        depthStencilView.image = shadowDepthImage;

        if (vkCreateImageView(bananDevice.device(), &depthStencilView, nullptr, &shadowDepthImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow depth image view");
        }

        VkImageView attachments[2];
        attachments[0] = shadowColorImageView;
        attachments[1] = shadowDepthImageView;

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

    VkDescriptorImageInfo *BananShadowMapper::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        VkDescriptorImageInfo info{};
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        info.imageView = shadowDepthCubemapImageView;
        info.sampler = shadowDepthCubemapImageSampler;
        return &info;
    }

    VkRenderPass BananShadowMapper::getRenderPass() {
        return renderPass;
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

        barrier.image = shadowColorImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        VkImageSubresourceRange cubeFaceSubresourceRange = {};
        cubeFaceSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        cubeFaceSubresourceRange.baseMipLevel = 0;
        cubeFaceSubresourceRange.levelCount = 1;
        cubeFaceSubresourceRange.baseArrayLayer = faceindex;
        cubeFaceSubresourceRange.layerCount = 1;

        barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        barrier.image = shadowDepthCubemapImage;
        barrier.subresourceRange = cubeFaceSubresourceRange;

        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

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

        vkCmdCopyImage(commandBuffer, shadowColorImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, shadowDepthCubemapImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        barrier.image = shadowColorImage;
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

        barrier.image = shadowDepthCubemapImage;
        barrier.subresourceRange.baseArrayLayer = faceindex;

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    }
}