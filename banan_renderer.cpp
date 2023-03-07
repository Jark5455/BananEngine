//
// Created by yashr on 1/3/22.
//

#include "banan_renderer.h"

#include <stdexcept>
#include <array>

namespace Banan {
    BananRenderer::BananRenderer(BananWindow &window, BananDevice &device) : bananWindow{window}, bananDevice{device} {
        recreateSwapChain();
        createCommandBuffers();
    }

    BananRenderer::~BananRenderer() {
        freeCommandBuffers();
    }

    bool BananRenderer::isFrameInProgress() const {
        return isFrameStarted;
    }

    VkCommandBuffer BananRenderer::getCurrentCommandBuffer() const {
        assert(isFrameStarted && "Cannot get Command buffer when frame is not in progress");
        return commandBuffers[currentFrameIndex];
    }

    VkRenderPass BananRenderer::getGeometryRenderPass() const {
        return bananSwapChain->getGeometryRenderpass();
    }

    VkRenderPass BananRenderer::getEdgeDetectionRenderPass() const {
        return bananSwapChain->getEdgeDetectionRenderPass();
    }

    VkRenderPass BananRenderer::getBlendWeightRenderPass() const {
        return bananSwapChain->getBlendWeightRenderpass();
    }

    VkRenderPass BananRenderer::getResolveRenderPass() const {
        return bananSwapChain->getResolveRenderpass();
    }

    void BananRenderer::createCommandBuffers() {
        commandBuffers.resize(BananSwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = bananDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(bananDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers");
        }
    }

    void BananRenderer::freeCommandBuffers() {
        vkFreeCommandBuffers(bananDevice.device(), bananDevice.getCommandPool(), commandBuffers.size(),
                             commandBuffers.data());
        commandBuffers.clear();
    }

    void BananRenderer::recreateSwapChain() {
        auto extent = bananWindow.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = bananWindow.getExtent();

            // process any queued events
            SDL_Event event;
            SDL_WaitEvent(&event);
        }

        vkDeviceWaitIdle(bananDevice.device());
        if (bananSwapChain == nullptr) {
            bananSwapChain = std::make_unique<BananSwapChain>(bananDevice, extent, nullptr);
        } else {
            std::shared_ptr<BananSwapChain> oldSwapChain = std::move(bananSwapChain);
            bananSwapChain = std::make_unique<BananSwapChain>(bananDevice, extent, oldSwapChain);
            assert(bananSwapChain->imageCount() == oldSwapChain->imageCount() && "Swap chain image count has changed!");
        }

        //TODO recreate pipelines at end of this function call
    }

    VkCommandBuffer BananRenderer::beginFrame() {
        assert(!isFrameStarted && "can't begin frame while another frame is already in progress");

        auto result = bananSwapChain->acquireNextImage(&currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image");
        }

        isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to start recording command buffer");
        }

        return commandBuffer;
    }

    void BananRenderer::endFrame() {
        assert(isFrameStarted && "Cannot end frame if frame has not been started");
        auto commandBuffer = getCurrentCommandBuffer();

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("unable to stop command buffer recording");
        }

        auto result = bananSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % BananSwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void BananRenderer::endRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "Cant end render pass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer that is different to the current frame");

        vkCmdEndRenderPass(commandBuffer);
    }

    void BananRenderer::beginGeometryRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "Cant begin render pass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer that is different to the current frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = bananSwapChain->getGeometryRenderpass();
        renderPassInfo.framebuffer = bananSwapChain->getGeometryFramebuffer();

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = bananSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 4> clearValues{};
        clearValues[0].depthStencil = {1.0f, 0};
        clearValues[1].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[2].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[3].color = {{0.1f, 0.1f, 0.1f, 1.0f}};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(bananSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(bananSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, bananSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void BananRenderer::beginEdgeDetectionRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "Cant begin render pass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer that is different to the current frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = bananSwapChain->getEdgeDetectionRenderPass();
        renderPassInfo.framebuffer = bananSwapChain->getEdgeDetectionFramebuffer();

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = bananSwapChain->getSwapChainExtent();

        VkClearValue clearValue = {{{0.0f, 0.0f}}};

        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(bananSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(bananSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, bananSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void BananRenderer::beginBlendWeightRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "Cant begin render pass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer that is different to the current frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = bananSwapChain->getBlendWeightRenderpass();
        renderPassInfo.framebuffer = bananSwapChain->getBlendWeightFramebuffer();

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = bananSwapChain->getSwapChainExtent();

        VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(bananSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(bananSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, bananSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void BananRenderer::beginResolveRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "Cant begin render pass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer that is different to the current frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = bananSwapChain->getResolveRenderpass();
        renderPassInfo.framebuffer = bananSwapChain->getResolveFramebuffer((int) currentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = bananSwapChain->getSwapChainExtent();

        VkClearValue clearValue = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(bananSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(bananSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, bananSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    int BananRenderer::getFrameIndex() const {
        assert(isFrameStarted && "Cannot get frame index when frame is not in progress");
        return currentFrameIndex;
    }

    float BananRenderer::getAspectRatio() const {
        return bananSwapChain->extentAspectRatio();
    }

    std::vector<VkDescriptorImageInfo> BananRenderer::getGBufferDescriptorInfo() {

        GBufferInfo.clear();

        GBufferInfo.push_back(bananSwapChain->gbuffer()[0]->descriptorInfo());
        GBufferInfo.push_back(bananSwapChain->gbuffer()[1]->descriptorInfo());
        GBufferInfo.push_back(bananSwapChain->gbuffer()[2]->descriptorInfo());

        return GBufferInfo;
    }

    VkDescriptorImageInfo BananRenderer::getGeometryDescriptorInfo() {
        return bananSwapChain->geometry()->descriptorInfo();
    }

    VkDescriptorImageInfo BananRenderer::getEdgeDescriptorInfo() {
        return bananSwapChain->edge()->descriptorInfo();
    }

    VkDescriptorImageInfo BananRenderer::getBlendWeightDescriptorInfo() {
        return bananSwapChain->blend()->descriptorInfo();
    }
}