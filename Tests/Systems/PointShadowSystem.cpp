//
// Created by yashr on 4/5/23.
//

#include <stdexcept>
#include "PointShadowSystem.h"

namespace Banan {

    PointShadowSystem::PointShadowSystem(Banan::BananDevice &device, Banan::BananGameObjectManager &manager) : bananDevice{device}, bananGameObjectManager{manager} {
        createRenderpass();
        createFramebuffers();
    }

    PointShadowSystem::~PointShadowSystem() {
        for (auto &kv : framebuffers) {
            vkDestroyFramebuffer(bananDevice.device(), kv.second, nullptr);
        }

        vkDestroyRenderPass(bananDevice.device(), shadowRenderpass, nullptr);
    }

    void Banan::PointShadowSystem::createRenderpass() {
        depthFormat = bananDevice.findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},VK_IMAGE_TILING_OPTIMAL,VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        std::vector<VkAttachmentDescription> attachments{2};
        attachments[0].format = VK_FORMAT_R16G16B16A16_USCALED;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        attachments[1].format = depthFormat;
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentReference attachmentReference{};
        attachmentReference.attachment = 0;
        attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentReference{};
        attachmentReference.attachment = 1;
        attachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &attachmentReference;
        subpass.pDepthStencilAttachment = &depthAttachmentReference;

        std::vector<VkSubpassDependency> dependencies{3};

        // ensure writeability
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = 0;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].dstSubpass = 0;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[1].srcAccessMask = 0;
        dependencies[1].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // shader read only
        dependencies[2].srcSubpass = 0;
        dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[2].dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
        dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[2].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // write to 6 layers
        const uint32_t viewMask = 0b00111111;

        // none of the views overlap, therefore null
        const uint32_t correlationMask = 0;

        VkRenderPassMultiviewCreateInfoKHR multi{};
        multi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO_KHR;
        multi.subpassCount = 1;
        multi.dependencyCount = dependencies.size();
        multi.pViewMasks = &viewMask;
        multi.correlationMaskCount = 0;
        multi.pCorrelationMasks = &correlationMask;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = attachments.size();
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = dependencies.size();
        renderPassInfo.pDependencies = dependencies.data();
        renderPassInfo.pNext = &multi;

        if (vkCreateRenderPass(bananDevice.device(), &renderPassInfo, nullptr, &shadowRenderpass) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create shadow renderpass");
        }
    }

    void PointShadowSystem::createFramebuffers() {
        for (auto &kv : bananGameObjectManager.getGameObjects()) {
            if (kv.second.pointLight != nullptr && kv.second.pointLight->castsShadows) {
                std::shared_ptr<BananCubemap> cubemap = std::make_shared<BananCubemap>(bananDevice, 1024, 1, VK_FORMAT_R16G16B16A16_USCALED, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                std::shared_ptr<BananCubemap> depthCubemap = std::make_shared<BananCubemap>(bananDevice, 1024, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                auto view = cubemap->descriptorInfo().imageView;

                VkFramebuffer framebuffer;
                VkFramebufferCreateInfo framebufferInfo = {};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = shadowRenderpass;
                framebufferInfo.attachmentCount = 1;
                framebufferInfo.pAttachments = &view;
                framebufferInfo.width = 1024;
                framebufferInfo.height = 1024;
                framebufferInfo.layers = 6;

                if (vkCreateFramebuffer(bananDevice.device(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create shadow framebuffer!");
                }

                cubemaps.emplace(kv.first, cubemap);
                depthcubemaps.emplace(kv.first, depthCubemap);
                framebuffers.emplace(kv.first, framebuffer);
            }
        }
    }

    void PointShadowSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> layouts) {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void PointShadowSystem::createPipeline() {

    }
}