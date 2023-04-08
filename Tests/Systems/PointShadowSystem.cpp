//
// Created by yashr on 4/5/23.
//

#include <banan_camera.h>

#include <stdexcept>

#include "PointShadowSystem.h"

namespace Banan {

    PointShadowSystem::PointShadowSystem(Banan::BananDevice &device, Banan::BananGameObjectManager &manager, std::vector<VkDescriptorSetLayout> layouts) : bananDevice{device}, bananGameObjectManager{manager} {
        createRenderpass();
        createFramebuffers();

        createMatrixBuffers();
        createDescriptors();

        createPipelineLayout(layouts);
        createPipeline();
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
        attachments[0].format = VK_FORMAT_R16G16B16A16_UNORM;
        attachments[0].samples = VK_SAMPLE_COUNT_4_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        attachments[1].format = depthFormat;
        attachments[1].samples = VK_SAMPLE_COUNT_4_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference attachmentReference{};
        attachmentReference.attachment = 0;
        attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentReference{};
        depthAttachmentReference.attachment = 1;
        depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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
        multi.pViewMasks = &viewMask;
        multi.correlationMaskCount = 1;
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
                std::shared_ptr<BananCubemap> cubemap = std::make_shared<BananCubemap>(bananDevice, 1024, 1, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                std::shared_ptr<BananCubemap> depthCubemap = std::make_shared<BananCubemap>(bananDevice, 1024, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                std::vector<VkImageView> attachments = {cubemap->arrayDescriptorInfo().imageView, depthCubemap->arrayDescriptorInfo().imageView};

                VkFramebuffer framebuffer;
                VkFramebufferCreateInfo framebufferInfo = {};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = shadowRenderpass;
                framebufferInfo.attachmentCount = attachments.size();
                framebufferInfo.pAttachments = attachments.data();
                framebufferInfo.width = 1024;
                framebufferInfo.height = 1024;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(bananDevice.device(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create shadow framebuffer!");
                }

                assert(cubemaps.size() == depthcubemaps.size() && "number of depth buffers differ from number of color buffers");
                cubemapalias.emplace(kv.first, cubemaps.size());
                cubemaps.push_back(cubemap);
                depthcubemaps.push_back(depthCubemap);

                framebuffers.emplace(kv.first, framebuffer);
            }
        }
    }

    void PointShadowSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> layouts) {

        layouts.push_back(shadowMatrixSetLayout->getDescriptorSetLayout());

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow pipeline layout!");
        }
    }

    void PointShadowSystem::createPipeline() {
        assert(pipelineLayout != nullptr && "pipelineLayout must be created before pipeline");

        // why not use the existing variables HEIGHT or WIDTH? - On high pixel density displays such as apples "retina" display these values are incorrect, but the swap chain corrects these values
        PipelineConfigInfo pipelineConfig{};
        BananPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();
        BananPipeline::shadowPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = shadowRenderpass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipelineConfig.subpass = 0;

        bananPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/shadow.vert.spv", "shaders/shadow.frag.spv", pipelineConfig);
    }

    void PointShadowSystem::beginShadowRenderpass(VkCommandBuffer commandBuffer, BananGameObject::id_t index) {
        VkClearValue clearValues[2];
        clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
        clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        // Reuse render pass from example pass
        renderPassBeginInfo.renderPass = shadowRenderpass;
        renderPassBeginInfo.framebuffer = framebuffers.at(index);
        renderPassBeginInfo.renderArea.extent.width = 1024;
        renderPassBeginInfo.renderArea.extent.height = 1024;
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0;
        viewport.y = 1024;
        viewport.width = 1024;
        viewport.height = -1024;
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

    void PointShadowSystem::endShadowRenderpass(VkCommandBuffer commandBuffer) {
        vkCmdEndRenderPass(commandBuffer);
    }

    void PointShadowSystem::render(BananFrameInfo frameInfo) {
        for (auto &kv : framebuffers) {
            beginShadowRenderpass(frameInfo.commandBuffer, kv.first);

            bananPipeline->bind(frameInfo.commandBuffer);

            for (auto &objectkv : bananGameObjectManager.getGameObjects()) {
                std::vector<VkDescriptorSet> sets = {frameInfo.globalDescriptorSet, frameInfo.gameObjectDescriptorSet, shadowMatrixDescriptorSets[frameInfo.frameIndex]};
                std::vector<uint32_t> offsets = {objectkv.first * (unsigned int) frameInfo.gameObjectManager.getGameObjectBufferAlignmentSize(frameInfo.frameIndex),
                                                 (unsigned int) cubemapalias.at(kv.first) * (unsigned int) matriceBuffers[frameInfo.frameIndex]->getAlignmentSize()};

                vkCmdBindDescriptorSets(frameInfo.commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelineLayout,0,sets.size(),sets.data(),offsets.size(),offsets.data());

                objectkv.second.model->bindPosition(frameInfo.commandBuffer);
                objectkv.second.model->draw(frameInfo.commandBuffer);
            }

            endShadowRenderpass(frameInfo.commandBuffer);
        }
    }

    void PointShadowSystem::generateMatrices(BananFrameInfo frameInfo) {
        BananCamera shadowCamera{};
        shadowCamera.setOrthographicProjection(-1.f, 1.f, -1.f, 1.f, -1.f, 2.f);

        for (auto &kv : bananGameObjectManager.getGameObjects()) {
            if (kv.second.pointLight == nullptr || !kv.second.pointLight->castsShadows) continue;

            PointShadowSystem::ShadowCubemapMatrices mat{};
            mat.projectionMatrix = shadowCamera.getProjection();

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

            matriceBuffers[frameInfo.frameIndex]->writeToIndex(&mat, cubemapalias.at(kv.first));
            matriceBuffers[frameInfo.frameIndex]->flushIndex(cubemapalias.at(kv.first));
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
                .setMaxSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, BananSwapChain::MAX_FRAMES_IN_FLIGHT)
                .build();

        shadowMatrixSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1)
                .build();

        for (size_t i = 0; i < shadowMatrixDescriptorSets.size(); i++) {
            BananDescriptorWriter shadowWriter(*shadowMatrixSetLayout, *shadowPool);
            auto bufferInfo = matriceBuffers[i]->descriptorInfo();
            shadowWriter.writeBuffer(0, bufferInfo);
            shadowWriter.build(shadowMatrixDescriptorSets[i]);
        }
    }
}