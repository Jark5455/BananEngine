//
// Created by yashr on 4/5/23.
//

#include <banan_camera.h>

#include <stdexcept>

#include "PointShadowSystem.h"

namespace Banan {

    PointShadowSystem::PointShadowSystem(Banan::BananDevice &device, Banan::BananGameObjectManager &manager, std::vector<VkDescriptorSetLayout> layouts) : bananDevice{device}, bananGameObjectManager{manager} {
        vkCreateRenderPass2Khr = (PFN_vkCreateRenderPass2KHR) vkGetDeviceProcAddr(bananDevice.device(), "vkCreateRenderPass2KHR");

        createDepthPrepass();
        createQuantizationPass();
        createDepthFramebuffers();
        createQuantizationFramebuffers();

        createBlurResources();
        createMatrixBuffers();
        createDescriptors();

        createDepthPipelineLayout(layouts);
        createDepthPipeline();

        createQuantPipelineLayout({shadowQuantizationSetLayout->getDescriptorSetLayout()});
        createQuantPipeline();

        createShadowMapFilteringPipelineLayout({shadowMapSetLayout->getDescriptorSetLayout()});
        createShadowMapFilteringPipeline();
    }

    PointShadowSystem::~PointShadowSystem() {
        vkDestroyPipelineLayout(bananDevice.device(), quantPipelineLayout, nullptr);
        vkDestroyPipelineLayout(bananDevice.device(), depthPipelineLayout, nullptr);

        for (auto &kv : depthFrameBuffers) {
            vkDestroyFramebuffer(bananDevice.device(), kv.second, nullptr);
        }

        for (auto &kv : quantizationFrameBuffers) {
            vkDestroyFramebuffer(bananDevice.device(), kv.second, nullptr);
        }

        vkDestroyRenderPass(bananDevice.device(), depthPrepass, nullptr);
        vkDestroyRenderPass(bananDevice.device(), quantizationPass, nullptr);
    }

    void Banan::PointShadowSystem::createDepthPrepass() {

        std::array<VkAttachmentDescription2KHR, 2> attachments{};

        attachments[0].sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
        attachments[0].format = VK_FORMAT_D32_SFLOAT;
        attachments[0].samples = VK_SAMPLE_COUNT_4_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments[0].flags = 0;

        attachments[1].sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
        attachments[1].format = VK_FORMAT_D32_SFLOAT;
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachments[1].flags = 0;

        VkAttachmentReference2KHR depthAttachmentReference{};
        depthAttachmentReference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
        depthAttachmentReference.attachment = 0;
        depthAttachmentReference.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference2KHR depthResolveAttachmentReference{};
        depthResolveAttachmentReference.attachment = 1;
        depthResolveAttachmentReference.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthResolveAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescriptionDepthStencilResolveKHR depthStencilResolve{};
        depthStencilResolve.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE_KHR;
        depthStencilResolve.stencilResolveMode = VK_RESOLVE_MODE_NONE_KHR;
        depthStencilResolve.depthResolveMode = VK_RESOLVE_MODE_AVERAGE_BIT_KHR;
        depthStencilResolve.pDepthStencilResolveAttachment = &depthResolveAttachmentReference;

        // write to 2 layers
        const uint32_t viewMask = 0b00000011;

        // none of the views overlap, therefore null
        const uint32_t correlationMask = 0;

        VkSubpassDescription2KHR subpassDescription{};
        subpassDescription.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2_KHR;
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 0;
        subpassDescription.inputAttachmentCount = 0;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pColorAttachments = nullptr;
        subpassDescription.pInputAttachments = nullptr;
        subpassDescription.pPreserveAttachments = nullptr;
        subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
        subpassDescription.pResolveAttachments = &depthResolveAttachmentReference;
        subpassDescription.flags = 0;
        subpassDescription.viewMask = viewMask;
        subpassDescription.pNext = &depthStencilResolve;

        std::vector<VkSubpassDependency2KHR> dependencies{2};

        // ensure write
        dependencies[0].sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2_KHR;
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = 0;
        dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies[0].viewOffset = 0;

        // make depth readable
        dependencies[1].sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2_KHR;
        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = 1;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies[1].viewOffset = 0;

        VkRenderPassCreateInfo2KHR renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR;
        renderPassCreateInfo.attachmentCount = attachments.size();
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;
        renderPassCreateInfo.dependencyCount = dependencies.size();
        renderPassCreateInfo.pDependencies = dependencies.data();
        renderPassCreateInfo.correlatedViewMaskCount = 1;
        renderPassCreateInfo.pCorrelatedViewMasks = &correlationMask;

        if (vkCreateRenderPass2Khr(bananDevice.device(), &renderPassCreateInfo, nullptr, &depthPrepass) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create shadow renderpass");
        }
    }

    void Banan::PointShadowSystem::createQuantizationPass() {

        VkAttachmentDescription2KHR colorAttachment;
        colorAttachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
        colorAttachment.format = VK_FORMAT_R16G16B16A16_UNORM;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        colorAttachment.flags = 0;

        VkAttachmentReference2KHR colorAttachmentReference{};
        colorAttachmentReference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // write to 2 layers
        const uint32_t viewMask = 0b00000011;

        // none of the views overlap, therefore null
        const uint32_t correlationMask = 0;

        VkSubpassDescription2KHR subpassDescription{};
        subpassDescription.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2_KHR;
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.inputAttachmentCount = 0;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pColorAttachments = &colorAttachmentReference;
        subpassDescription.pInputAttachments = nullptr;
        subpassDescription.pPreserveAttachments = nullptr;
        subpassDescription.pDepthStencilAttachment = nullptr;
        subpassDescription.pResolveAttachments = nullptr;
        subpassDescription.flags = 0;
        subpassDescription.viewMask = viewMask;
        subpassDescription.pNext = nullptr;

        std::vector<VkSubpassDependency2KHR> dependencies{2};

        // ensure write
        dependencies[0].sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2_KHR;
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = 0;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies[0].viewOffset = 0;

        // make depth readable
        dependencies[1].sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2_KHR;
        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = 1;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies[1].viewOffset = 0;

        VkRenderPassCreateInfo2KHR renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &colorAttachment;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;
        renderPassCreateInfo.dependencyCount = dependencies.size();
        renderPassCreateInfo.pDependencies = dependencies.data();
        renderPassCreateInfo.correlatedViewMaskCount = 1;
        renderPassCreateInfo.pCorrelatedViewMasks = &correlationMask;

        if (vkCreateRenderPass2Khr(bananDevice.device(), &renderPassCreateInfo, nullptr, &quantizationPass) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create shadow renderpass");
        }
    }

    void PointShadowSystem::createDepthFramebuffers() {
        for (auto &kv : bananGameObjectManager.getGameObjects()) {

            // skip if not pointlight
            if (kv.second.pointLight == nullptr || !kv.second.pointLight->castsShadows) continue;

            // skip if already allocated
            if (depthFrameBuffers.find(kv.first) != depthFrameBuffers.end()) continue;

            std::shared_ptr<BananImage> depthImage = std::make_shared<BananImage>(bananDevice, 1024, 1024, 1, 2, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            std::shared_ptr<BananImage> depthImageResolve = std::make_shared<BananImage>(bananDevice, 1024, 1024, 1, 2, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            std::array<VkImageView *, 2> attachments{};
            attachments[0] = (VkImageView *) depthImage->getImageView();
            attachments[1] = (VkImageView *) depthImageResolve->getImageView();

            VkFramebuffer framebuffer;
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = depthPrepass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments = (VkImageView *) attachments.data();
            framebufferInfo.width = 1024;
            framebufferInfo.height = 1024;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(bananDevice.device(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to create shadow framebuffer!");
            }

            assert(depthFrameBuffers.size() == depthFramebufferImages.size());
            assert(depthFrameBuffers.size() == depthFramebufferResolveImages.size());

            depthFrameBuffers.emplace(kv.first, framebuffer);
            depthFramebufferImages.emplace(kv.first, depthImage);
            depthFramebufferResolveImages.emplace(kv.first, depthImageResolve);
        }
    }

    void PointShadowSystem::createQuantizationFramebuffers() {
        for (auto &kv : bananGameObjectManager.getGameObjects()) {

            // skip if not pointlight
            if (kv.second.pointLight == nullptr || !kv.second.pointLight->castsShadows) continue;
            // skip if already allocated
            if (quantizationFrameBuffers.find(kv.first) != quantizationFrameBuffers.end()) continue;

            std::shared_ptr<BananImage> quantizationImage = std::make_shared<BananImage>(bananDevice, 1024, 1024, 1, 2, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            VkFramebuffer framebuffer;
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = depthPrepass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = (VkImageView *) (quantizationImage->getImageView());
            framebufferInfo.width = 1024;
            framebufferInfo.height = 1024;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(bananDevice.device(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to create shadow framebuffer!");
            }

            assert(quantizationFrameBuffers.size() == quantizedImages.size());

            quantizationFrameBuffers.emplace(kv.first, framebuffer);
            quantizedImages.emplace(kv.first, quantizationImage);
        }
    }

    void PointShadowSystem::createBlurResources() {
        for (auto &kv : bananGameObjectManager.getGameObjects()) {

            // skip if not pointlight
            if (kv.second.pointLight == nullptr || !kv.second.pointLight->castsShadows) continue;

            if (blurredImages.find(kv.first) == blurredImages.end()) {
                std::shared_ptr<BananImage> blurredImage = std::make_shared<BananImage>(bananDevice, 1024, 1024, 1, 2, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                blurredImages.emplace(kv.first, blurredImage);
            }

            if (shadowMaps.find(kv.first) == shadowMaps.end()) {
                std::shared_ptr<BananImage> shadowMap = std::make_shared<BananImage>(bananDevice, 1024, 1024, 1, 2, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                shadowMaps.emplace(kv.first, shadowMap);
            }
        }
    }

    void PointShadowSystem::createDepthPipelineLayout(std::vector<VkDescriptorSetLayout> layouts) {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &depthPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow pipeline layout!");
        }
    }

    void PointShadowSystem::createDepthPipeline() {
        assert(depthPipelineLayout != nullptr && "pipelineLayout must be created before pipeline");

        // why not use the existing variables HEIGHT or WIDTH? - On high pixel density displays such as apples "retina" display these values are incorrect, but the swap chain corrects these values
        PipelineConfigInfo pipelineConfig{};
        BananPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();
        BananPipeline::shadowPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = depthPrepass;
        pipelineConfig.pipelineLayout = depthPipelineLayout;
        pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
        pipelineConfig.rasterizationInfo.depthClampEnable = VK_FALSE;
        pipelineConfig.subpass = 0;

        depthPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/shadow.vert.spv", "shaders/shadow.frag.spv", pipelineConfig);
    }

    void PointShadowSystem::createQuantPipelineLayout(std::vector<VkDescriptorSetLayout> layouts) {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &quantPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow pipeline layout!");
        }
    }

    void PointShadowSystem::createQuantPipeline() {
        assert(quantPipelineLayout != nullptr && "pipelineLayout must be created before pipeline");

        // why not use the existing variables HEIGHT or WIDTH? - On high pixel density displays such as apples "retina" display these values are incorrect, but the swap chain corrects these values
        PipelineConfigInfo pipelineConfig{};
        BananPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.renderPass = quantizationPass;
        pipelineConfig.pipelineLayout = quantPipelineLayout;
        pipelineConfig.subpass = 0;

        quantPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/quant.vert.spv", "shaders/quant.frag.spv", pipelineConfig);
    }

    void PointShadowSystem::createShadowMapFilteringPipelineLayout(std::vector<VkDescriptorSetLayout> layouts) {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &shadowMapFilteringPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow pipeline layout!");
        }
    }

    void PointShadowSystem::createShadowMapFilteringPipeline() {
        assert(shadowMapFilteringPipelineLayout != nullptr && "pipelineLayout must be created before pipeline");

        // why not use the existing variables HEIGHT or WIDTH? - On high pixel density displays such as apples "retina" display these values are incorrect, but the swap chain corrects these values
        PipelineConfigInfo pipelineConfig{};
        pipelineConfig.pipelineLayout = shadowMapFilteringPipelineLayout;

        shadowMapFilteringPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/calc_gaussian_blur.comp.spv", pipelineConfig);
    }

    void PointShadowSystem::beginDepthPrepass(VkCommandBuffer commandBuffer, BananGameObject::id_t index) {
        std::array<VkClearValue, 1> clearValues{};
        clearValues[0].depthStencil = { 1.f, 0 };

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = depthPrepass;
        renderPassBeginInfo.framebuffer = quantizationFrameBuffers.at(index);
        renderPassBeginInfo.renderArea.extent.width = 1024;
        renderPassBeginInfo.renderArea.extent.height = 1024;
        renderPassBeginInfo.clearValueCount = clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = 1024;
        viewport.height = 1024;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.extent.width = 1024;
        scissor.extent.height = 1024;
        scissor.offset.x = 0;
        scissor.offset.y = 0;

        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void PointShadowSystem::endRenderPass(VkCommandBuffer commandBuffer) {
        vkCmdEndRenderPass(commandBuffer);
    }

    void PointShadowSystem::render(BananFrameInfo frameInfo) {

        // recreate resources if num point lights changed
        if (bananGameObjectManager.numPointLights() != matriceBuffers[frameInfo.frameIndex]->getInstanceCount()) {

            assert(quantizationFrameBuffers.size() == depthFrameBuffers.size());

            createDepthFramebuffers();
            createQuantizationFramebuffers();
            createBlurResources();
            createMatrixBuffers();
            createDescriptors();
        }

//        for (auto &kv : framebuffers) {
//            beginShadowRenderpass(frameInfo.commandBuffer, kv.first);
//
//            depthPipeline->bind(frameInfo.commandBuffer);
//
//            for (auto &objectkv : bananGameObjectManager.getGameObjects()) {
//                if (objectkv.second.model == nullptr) continue;
//
//                std::vector<VkDescriptorSet> sets = {frameInfo.globalDescriptorSet, frameInfo.gameObjectDescriptorSet, shadowMatrixDescriptorSets[frameInfo.frameIndex]};
//                std::vector<uint32_t> offsets = {objectkv.first * (unsigned int) frameInfo.gameObjectManager.getGameObjectBufferAlignmentSize(frameInfo.frameIndex), (unsigned int) cubemapalias.at(kv.first) * (unsigned int) matriceBuffers[frameInfo.frameIndex]->getAlignmentSize()};
//
//                vkCmdBindDescriptorSets(frameInfo.commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,depthPipelineLayout,0,sets.size(),sets.data(),offsets.size(),offsets.data());
//
//                objectkv.second.model->bindPosition(frameInfo.commandBuffer);
//                objectkv.second.model->draw(frameInfo.commandBuffer);
//            }
//
//            vkCmdNextSubpass(frameInfo.commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
//            quantPipeline->bind(frameInfo.commandBuffer);
//
//            std::vector<VkDescriptorSet> sets = {quantizationDescriptorSets.at(kv.first).at(frameInfo.frameIndex)};
//
//            vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, quantPipelineLayout, 0, sets.size(), sets.data(), 0, nullptr);
//            vkCmdDraw(frameInfo.commandBuffer, 3, 1, 0, 0);
//
//            endShadowRenderpass(frameInfo.commandBuffer);
//        }
    }

    void PointShadowSystem::generateMatrices(BananFrameInfo frameInfo) {
        BananCamera shadowCamera{};
        shadowCamera.setPerspectiveProjection(glm::pi<float>() / 2.0, 1, 0.1f, 1024.f);

        for (auto &kv : bananGameObjectManager.getGameObjects()) {
            if (kv.second.pointLight == nullptr || !kv.second.pointLight->castsShadows) continue;

            PointShadowSystem::ShadowCubemapMatrices mat{};
            mat.projectionMatrix = shadowCamera.getProjection();
            mat.invProjectionMatrix = shadowCamera.getInverseProjection();

            // POSITIVE_X
            shadowCamera.setViewYXZ(kv.second.transform.translation, {glm::radians(180.f), glm::radians(270.f), 0.f});
            mat.viewMatrices[0] = shadowCamera.getView();
            mat.invViewMatrices[0] = shadowCamera.getInverseView();

            // NEGATIVE_X
            shadowCamera.setViewYXZ(kv.second.transform.translation, {glm::radians(180.f), glm::radians(90.f), 0.f});
            mat.viewMatrices[1] = shadowCamera.getView();
            mat.invViewMatrices[1] = shadowCamera.getInverseView();

            // POSITIVE_Y
            shadowCamera.setViewYXZ(kv.second.transform.translation, {glm::radians(270.f), 0.f, glm::radians(180.f)});
            mat.viewMatrices[2] = shadowCamera.getView();
            mat.invViewMatrices[2] = shadowCamera.getInverseView();

            // NEGATIVE_Y
            shadowCamera.setViewYXZ(kv.second.transform.translation, {glm::radians(90.f), 0.f, glm::radians(180.f)});
            mat.viewMatrices[3] = shadowCamera.getView();
            mat.invViewMatrices[3] = shadowCamera.getInverseView();

            // POSITIVE_Z
            shadowCamera.setViewYXZ(kv.second.transform.translation, {0.f, 0.f, glm::radians(180.f)});
            mat.viewMatrices[4] = shadowCamera.getView();
            mat.invViewMatrices[4] = shadowCamera.getInverseView();

            // NEGATIVE_Z
            shadowCamera.setViewYXZ(kv.second.transform.translation, {glm::radians(180.f), 0.f, 0.f});
            mat.viewMatrices[5] = shadowCamera.getView();
            mat.invViewMatrices[5] = shadowCamera.getInverseView();

            matriceBuffers[frameInfo.frameIndex]->writeToIndex(&mat, kv.first);
            matriceBuffers[frameInfo.frameIndex]->flushIndex(kv.first);
        }
    }

    void PointShadowSystem::createMatrixBuffers() {
        matriceBuffers.resize(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto & uboBuffer : matriceBuffers) {
            uboBuffer = std::make_unique<BananBuffer>(bananDevice, sizeof(PointShadowSystem::ShadowCubemapMatrices), bananGameObjectManager.numPointLights(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, bananDevice.physicalDeviceProperties().limits.minUniformBufferOffsetAlignment);
            uboBuffer->map();
        }
    }

    void PointShadowSystem::createDescriptors() {
        shadowPool = BananDescriptorPool::Builder(bananDevice)
                .setMaxSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT * 5)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, BananSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, BananSwapChain::MAX_FRAMES_IN_FLIGHT * bananGameObjectManager.numPointLights() * 2 + 2)
                .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, BananSwapChain::MAX_FRAMES_IN_FLIGHT * bananGameObjectManager.numPointLights() * 4)
                .build();

        shadowUBOSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .build();

        shadowDepthSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
                .build();

        shadowQuantizationSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
                .build();

        shadowBlurSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
                .build();

        shadowMapSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
                .build();

        shadowUBODescriptorSets.resize(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < shadowUBODescriptorSets.size(); i++) {
            BananDescriptorWriter shadowWriter(*shadowUBOSetLayout, *shadowPool);
            auto bufferInfo = matriceBuffers[i]->descriptorInfo(matriceBuffers[i]->getInstanceSize());
            shadowWriter.writeBuffer(0, bufferInfo);
            shadowWriter.build(shadowUBODescriptorSets[i]);
        }

        std::vector<VkDescriptorImageInfo> depthMaps;
        depthMaps.resize(depthFramebufferResolveImages.size());

        std::vector<VkDescriptorImageInfo> quantizedMaps;
        quantizedMaps.resize(quantizedImages.size());

        std::vector<VkDescriptorImageInfo> blurredMaps;
        blurredMaps.resize(blurredImages.size());

        std::vector<VkDescriptorImageInfo> momentShadowMaps;
        momentShadowMaps.resize(shadowMaps.size());

        for (auto &kv : depthFramebufferResolveImages) {
            depthMaps.at(kv.first) = kv.second->descriptorInfo();
        }

        for (auto &kv : quantizedImages) {
            quantizedMaps.at(kv.first) = kv.second->descriptorInfo();
        }

        for (auto &kv : blurredImages) {
            blurredMaps.at(kv.first) = kv.second->descriptorInfo();
        }

        for (auto &kv : shadowMaps) {
            momentShadowMaps.at(kv.first) = kv.second->descriptorInfo();
        }

        shadowDepthDescriptorSets.resize(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto &set : shadowDepthDescriptorSets) {
            BananDescriptorWriter writer(*shadowMapSetLayout, *shadowPool);
            writer.writeImages(0, depthMaps);
            writer.build(set);
        }

        shadowQuantizationDescriptorSets.resize(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto &set : shadowQuantizationDescriptorSets) {
            BananDescriptorWriter writer(*shadowMapSetLayout, *shadowPool);
            writer.writeImages(0, quantizedMaps);
            writer.build(set);
        }

        shadowBlurPassDescriptorSets.resize(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto &set : shadowBlurPassDescriptorSets) {
            BananDescriptorWriter writer(*shadowMapSetLayout, *shadowPool);
            writer.writeImages(0, blurredMaps);
            writer.build(set);
        }

        shadowMapDescriptorSets.resize(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto &set : shadowMapDescriptorSets) {
            BananDescriptorWriter writer(*shadowMapSetLayout, *shadowPool);
            writer.writeImages(0, momentShadowMaps);
            writer.build(set);
        }
    }

    VkDescriptorSetLayout PointShadowSystem::getShadowMapsDescriptorSetLayout() {
        return shadowMapSetLayout->getDescriptorSetLayout();
    }

    VkDescriptorSet &PointShadowSystem::getShadowMapDescriptorSet(size_t frameIndex) {
        return shadowMapDescriptorSets[frameIndex];
    }
}