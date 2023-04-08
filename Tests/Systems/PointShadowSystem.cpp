//
// Created by yashr on 4/5/23.
//

#include <banan_camera.h>

#include <stdexcept>

#include "PointShadowSystem.h"

namespace Banan {

    PointShadowSystem::PointShadowSystem(Banan::BananDevice &device, Banan::BananGameObjectManager &manager, std::vector<VkDescriptorSetLayout> layouts) : bananDevice{device}, bananGameObjectManager{manager} {
        vkCreateRenderPass2KHR = (PFN_vkCreateRenderPass2KHR) vkGetDeviceProcAddr(bananDevice.device(), "vkCreateRenderPass2KHR");

        createRenderpass();
        createFramebuffers();

        createMatrixBuffers();
        createDescriptors();

        createDepthPipelineLayout(layouts);
        createDepthPipeline();

        createQuantPipelineLayout({quantizationSetLayout->getDescriptorSetLayout()});
        createQuantPipeline();
    }

    PointShadowSystem::~PointShadowSystem() {

        vkDestroyPipelineLayout(bananDevice.device(), quantPipelineLayout, nullptr);
        vkDestroyPipelineLayout(bananDevice.device(), depthPipelineLayout, nullptr);

        for (auto &kv : framebuffers) {
            vkDestroyFramebuffer(bananDevice.device(), kv.second, nullptr);
        }

        vkDestroyRenderPass(bananDevice.device(), shadowRenderpass, nullptr);
    }

    void Banan::PointShadowSystem::createRenderpass() {
        std::vector<VkAttachmentDescription2KHR> attachments{3};

        attachments[0].sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
        attachments[0].format = VK_FORMAT_D32_SFLOAT;
        attachments[0].samples = VK_SAMPLE_COUNT_4_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        attachments[0].pNext = nullptr;

        attachments[1].sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
        attachments[1].format = VK_FORMAT_D32_SFLOAT;
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachments[1].pNext = nullptr;

        attachments[2].sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2_KHR;
        attachments[2].format = VK_FORMAT_R16G16B16A16_UNORM;
        attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[2].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachments[2].pNext = nullptr;

        VkAttachmentReference2KHR depthAttachmentReference{};
        depthAttachmentReference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
        depthAttachmentReference.attachment = 0;
        depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachmentReference.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthAttachmentReference.pNext = nullptr;

        VkAttachmentReference2KHR resolveAttachmentReference{};
        resolveAttachmentReference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
        resolveAttachmentReference.attachment = 1;
        resolveAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        resolveAttachmentReference.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        resolveAttachmentReference.pNext = nullptr;

        VkAttachmentReference2KHR quantAttachmentReference{};
        resolveAttachmentReference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2_KHR;
        resolveAttachmentReference.attachment = 2;
        resolveAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        resolveAttachmentReference.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        resolveAttachmentReference.pNext = nullptr;

        VkSubpassDescriptionDepthStencilResolveKHR depthStencilResolveAttachment{};
        depthStencilResolveAttachment.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE_KHR;
        depthStencilResolveAttachment.pDepthStencilResolveAttachment = &resolveAttachmentReference;
        depthStencilResolveAttachment.depthResolveMode = VK_RESOLVE_MODE_AVERAGE_BIT_KHR;
        depthStencilResolveAttachment.stencilResolveMode = VK_RESOLVE_MODE_NONE_KHR;

        // write to 6 layers
        const uint32_t viewMask = 0b00111111;

        // none of the views overlap, therefore null
        const uint32_t correlationMask = 0;

        std::vector<VkSubpassDescription2KHR> subpasses{2};
        subpasses[0].sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2_KHR;
        subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpasses[0].pDepthStencilAttachment = &depthAttachmentReference;
        subpasses[0].inputAttachmentCount = 0;
        subpasses[0].pInputAttachments = nullptr;
        subpasses[0].colorAttachmentCount = 0;
        subpasses[0].pColorAttachments = nullptr;
        subpasses[0].preserveAttachmentCount = 0;
        subpasses[0].pPreserveAttachments = nullptr;
        subpasses[0].pResolveAttachments = nullptr;
        subpasses[0].viewMask = viewMask;
        subpasses[0].flags = 0;
        subpasses[0].pNext = &depthStencilResolveAttachment;

        subpasses[1].sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2_KHR;
        subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpasses[1].pDepthStencilAttachment = nullptr;
        subpasses[1].inputAttachmentCount = 1;
        subpasses[1].pInputAttachments = &resolveAttachmentReference;
        subpasses[1].colorAttachmentCount = 1;
        subpasses[1].pColorAttachments = &quantAttachmentReference;
        subpasses[1].preserveAttachmentCount = 0;
        subpasses[1].pPreserveAttachments = nullptr;
        subpasses[1].pResolveAttachments = nullptr;
        subpasses[1].viewMask = viewMask;
        subpasses[1].flags = 0;
        subpasses[1].pNext = nullptr;

        std::vector<VkSubpassDependency2KHR> dependencies{4};

        // ensure write access
        dependencies[0].sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2_KHR;
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = 0;
        dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies[0].pNext = nullptr;
        dependencies[0].viewOffset = 0;

        dependencies[1].sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2_KHR;
        dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].dstSubpass = 0;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].srcAccessMask = 0;
        dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies[1].pNext = nullptr;
        dependencies[1].viewOffset = 0;

        // make depth readable
        dependencies[2].sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2_KHR;
        dependencies[2].srcSubpass = 0;
        dependencies[2].dstSubpass = 1;
        dependencies[2].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[2].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[2].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[2].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
        dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies[2].pNext = nullptr;
        dependencies[2].viewOffset = 0;

        // make color readable
        dependencies[3].sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2_KHR;
        dependencies[3].srcSubpass = 1;
        dependencies[3].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[3].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[3].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[3].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[3].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[3].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies[3].pNext = nullptr;
        dependencies[3].viewOffset = 0;

        VkRenderPassMultiviewCreateInfoKHR multi{};
        multi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO_KHR;
        multi.subpassCount = 1;
        multi.pViewMasks = &viewMask;
        multi.correlationMaskCount = 1;
        multi.pCorrelationMasks = &correlationMask;

        VkRenderPassCreateInfo2KHR renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2_KHR;
        renderPassCreateInfo.attachmentCount = attachments.size();
        renderPassCreateInfo.pAttachments = attachments.data();
        renderPassCreateInfo.subpassCount = subpasses.size();
        renderPassCreateInfo.pSubpasses = subpasses.data();
        renderPassCreateInfo.dependencyCount = dependencies.size();
        renderPassCreateInfo.pDependencies = dependencies.data();
        renderPassCreateInfo.pNext = &multi;

        if (vkCreateRenderPass2KHR(bananDevice.device(), &renderPassCreateInfo, nullptr, &shadowRenderpass) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create shadow renderpass");
        }
    }

    void PointShadowSystem::createFramebuffers() {
        for (auto &kv : bananGameObjectManager.getGameObjects()) {
            if (kv.second.pointLight != nullptr && kv.second.pointLight->castsShadows) {
                std::shared_ptr<BananImageArray> depthCubemap = std::make_shared<BananImageArray>(bananDevice, 1024, 1024, 6, 1, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_4_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                std::shared_ptr<BananCubemap> resolveCubemap = std::make_shared<BananCubemap>(bananDevice, 1024, 1, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                std::shared_ptr<BananCubemap> quantCubemap = std::make_shared<BananCubemap>(bananDevice, 1024, 1, VK_FORMAT_R16G16B16A16_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                std::vector<VkImageView> attachments = {depthCubemap->descriptorInfo().imageView, resolveCubemap->cubemapDescriptorInfo().imageView, quantCubemap->cubemapDescriptorInfo().imageView};

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

                assert(depthFramebufferImages.size() == framebufferResolveCubemaps.size() && "number of resolve buffers differ from number of depth buffers");
                assert(depthFramebufferImages.size() == quantCubemaps.size() && "number of quant buffers differ from number of depth buffers");

                cubemapalias.emplace(kv.first, depthFramebufferImages.size());
                depthFramebufferImages.push_back(depthCubemap);
                framebufferResolveCubemaps.push_back(resolveCubemap);
                quantCubemaps.push_back(quantCubemap);

                framebuffers.emplace(kv.first, framebuffer);
            }
        }
    }

    void PointShadowSystem::createDepthPipelineLayout(std::vector<VkDescriptorSetLayout> layouts) {

        layouts.push_back(shadowMatrixSetLayout->getDescriptorSetLayout());

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
        pipelineConfig.renderPass = shadowRenderpass;
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
        pipelineConfig.renderPass = shadowRenderpass;
        pipelineConfig.pipelineLayout = depthPipelineLayout;
        pipelineConfig.subpass = 1;

        quantPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/quant.vert.spv", "shaders/quant.frag.spv", pipelineConfig);
    }

    void PointShadowSystem::beginShadowRenderpass(VkCommandBuffer commandBuffer, BananGameObject::id_t index) {
        std::array<VkClearValue, 4> clearValues{};
        clearValues[0].depthStencil = { 2.0f, 0 };
        clearValues[1].depthStencil = { 2.0f, 0 };
        clearValues[3].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = shadowRenderpass;
        renderPassBeginInfo.framebuffer = framebuffers.at(index);
        renderPassBeginInfo.renderArea.extent.width = 1024;
        renderPassBeginInfo.renderArea.extent.height = 1024;
        renderPassBeginInfo.clearValueCount = clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0;
        viewport.y = 1024;
        viewport.width = 1024;
        viewport.height = -1024;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 2.0f;

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

            depthPipeline->bind(frameInfo.commandBuffer);

            for (auto &objectkv : bananGameObjectManager.getGameObjects()) {
                if (objectkv.second.model == nullptr) continue;

                std::vector<VkDescriptorSet> sets = {frameInfo.globalDescriptorSet, frameInfo.gameObjectDescriptorSet, shadowMatrixDescriptorSets[frameInfo.frameIndex]};
                std::vector<uint32_t> offsets = {objectkv.first * (unsigned int) frameInfo.gameObjectManager.getGameObjectBufferAlignmentSize(frameInfo.frameIndex),
                                                 (unsigned int) cubemapalias.at(kv.first) * (unsigned int) matriceBuffers[frameInfo.frameIndex]->getAlignmentSize()};

                vkCmdBindDescriptorSets(frameInfo.commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,depthPipelineLayout,0,sets.size(),sets.data(),offsets.size(),offsets.data());

                objectkv.second.model->bindPosition(frameInfo.commandBuffer);
                objectkv.second.model->draw(frameInfo.commandBuffer);
            }

            vkCmdNextSubpass(frameInfo.commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
            quantPipeline->bind(frameInfo.commandBuffer);

            std::vector<VkDescriptorSet> sets = {quantizationDescriptorSets.at(kv.first).at(frameInfo.frameIndex)};
            vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, quantPipelineLayout, 0, sets.size(), sets.data(), 0, nullptr);
            vkCmdDraw(frameInfo.commandBuffer, 3, 1, 0, 0);

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

        quantPool = BananDescriptorPool::Builder(bananDevice)
                .setMaxSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT * bananGameObjectManager.numPointLights())
                .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, BananSwapChain::MAX_FRAMES_IN_FLIGHT * bananGameObjectManager.numPointLights())
                .build();

        shadowMatrixSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT)
                .build();

        quantizationSetLayout = BananDescriptorSetLayout::Builder(bananDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
                .build();

        shadowMatrixDescriptorSets.resize(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < shadowMatrixDescriptorSets.size(); i++) {
            BananDescriptorWriter shadowWriter(*shadowMatrixSetLayout, *shadowPool);
            auto bufferInfo = matriceBuffers[i]->descriptorInfo(matriceBuffers[i]->getInstanceSize());
            shadowWriter.writeBuffer(0, bufferInfo);
            shadowWriter.build(shadowMatrixDescriptorSets[i]);
        }

        for (auto &kv : bananGameObjectManager.getGameObjects()) {
            std::vector<VkDescriptorSet> &sets = quantizationDescriptorSets.at(kv.first);
            sets.resize(2);

            for (size_t i = 0; i < sets.size(); i++) {
                BananDescriptorWriter writer(*quantizationSetLayout, *quantPool);
                auto imageInfo = framebufferResolveCubemaps[cubemapalias.at(kv.first)]->cubemapDescriptorInfo();
                writer.writeImage(0, imageInfo);
                writer.build(quantizationDescriptorSets.at(kv.first).at(i));
            }
        }
    }
}