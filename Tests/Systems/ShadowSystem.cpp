//
// Created by yashr on 6/29/22.
//

#include "ShadowSystem.h"

#include <stdexcept>

namespace Banan {

    ShadowSystem::ShadowSystem(BananDevice &device, VkDescriptorSetLayout globalSetLayout) : bananDevice{device} {
        frameBufferDepthFormat = device.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},VK_IMAGE_TILING_OPTIMAL,VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        createShadowDepthResources();
        createShadowRenderPass();
        createPipelineLayout(globalSetLayout);
        createPipeline();
        createShadowFramebuffers();
    }

    void ShadowSystem::createShadowRenderPass() {
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

    void ShadowSystem::createShadowDepthResources() {
        VkFormat format = VK_FORMAT_R32_SFLOAT;

        // Cube map image description
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = format;
        imageCreateInfo.extent = { 1024, 1024, 1 };
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 6;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        bananDevice.createImageWithInfo(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shadowDepthCubemapImage, shadowDepthCubemapImageMemory);
        bananDevice.transitionImageLayout(shadowDepthCubemapImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 6);

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

        if (vkCreateSampler(bananDevice.device(), &samplerCreateInfo, nullptr, &shadowDepthCubemapImageSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow depth cubemap sampler");
        }

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = shadowDepthCubemapImage;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        imageViewCreateInfo.format = format;
        imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R };
        imageViewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        imageViewCreateInfo.subresourceRange.layerCount = 6;

        if (vkCreateImageView(bananDevice.device(), &imageViewCreateInfo, nullptr, &shadowDepthCubemapImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow depth cubemap image view");
        }
    }

    void ShadowSystem::createShadowFramebuffers() {
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

        bananDevice.createImageWithInfo(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shadowColorImage, shadowColorImageMemory);
        bananDevice.transitionImageLayout(shadowColorImage, VK_FORMAT_R32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, 1);

        VkImageViewCreateInfo colorImageViewCreateInfo{};
        colorImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        colorImageViewCreateInfo.format = VK_FORMAT_R32_SFLOAT;
        colorImageViewCreateInfo.flags = 0;
        colorImageViewCreateInfo.subresourceRange = {};
        colorImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        colorImageViewCreateInfo.subresourceRange.levelCount = 1;
        colorImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        colorImageViewCreateInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(bananDevice.device(), &colorImageViewCreateInfo, nullptr, &shadowColorImageView) != VK_SUCCESS) {
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

        bananDevice.createImageWithInfo(depthImageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shadowDepthImage, shadowDepthImageMemory);
        bananDevice.transitionImageLayout(shadowDepthImage, frameBufferDepthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 1);

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

    VkDescriptorImageInfo ShadowSystem::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
        VkDescriptorImageInfo info{};
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        info.imageView = shadowDepthCubemapImageView;
        info.sampler = shadowDepthCubemapImageSampler;
        return info;
    }

    void ShadowSystem::update(BananFrameInfo &frameInfo, GlobalUbo &ubo) {

    }

    void ShadowSystem::render(BananFrameInfo &frameInfo) {

    }

    void ShadowSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = 0;

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow pipeline layout!");
        }
    }

    void ShadowSystem::createPipeline() {
        assert(pipelineLayout != nullptr && "pipelineLayout must be created before pipeline");

        // why not use the existing variables HEIGHT or WIDTH? - On high pixel density displays such as apples "retina" display these values are incorrect, but the swap chain corrects these values
        PipelineConfigInfo pipelineConfig{};
        BananPipeline::shadowPipelineConfigInfo(pipelineConfig);

        bananPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/shadow.vert.spv", "shaders/shadow.frag.spv", pipelineConfig);
    }
}