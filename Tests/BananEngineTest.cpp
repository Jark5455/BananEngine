//
// Created by yashr on 12/4/21.
//

#include "BananEngineTest.h"

#include <stdexcept>

namespace Banan{

    BananEngineTest::BananEngineTest() {
        loadModels();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
    }

    BananEngineTest::~BananEngineTest() {
        vkDestroyPipelineLayout(bananDevice.device(), pipelineLayout, nullptr);
    }

    void BananEngineTest::run() {
        while(!bananWindow.windowShouldClose())
        {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(bananDevice.device());
    }

    void BananEngineTest::createPipelineLayout() {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void BananEngineTest::createPipeline() {
        // why not use the existing variables HEIGHT or WIDTH? - On high pixel density displays such as apples "retina" display these values are incorrect, but the swap chain corrects these values
        auto pipelineConfig = BananPipeline::defaultPipelineConfigInfo(bananSwapChain->width(), bananSwapChain->height());
        pipelineConfig.renderPass = bananSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        bananPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/triangle.vert.spv", "shaders/triangle.frag.spv", pipelineConfig);
    }

    void BananEngineTest::createCommandBuffers() {
        commandBuffers.resize(bananSwapChain->imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = bananDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(bananDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers");
        }
    }

    void BananEngineTest::drawFrame() {
        uint32_t imageIndex;
        auto result = bananSwapChain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }

        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image");
        }

        recordCommandBuffer(static_cast<int>(imageIndex));
        result = bananSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || bananWindow.wasWindowResized()) {
            bananWindow.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }

    void BananEngineTest::loadModels() {
        std::vector<BananModel::Vertex> vertices{
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };
        bananModel = std::make_unique<BananModel>(bananDevice, vertices);
    }

    void BananEngineTest::recreateSwapChain() {
        auto extent = bananWindow.getExtent();
        while(extent.width == 0 || extent.height == 0) {
            extent = bananWindow.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(bananDevice.device());
        if (bananSwapChain == nullptr) {
            bananSwapChain = std::make_unique<BananSwapChain>(bananDevice, extent, nullptr);
        } else {
            std::shared_ptr<BananSwapChain> oldSwapChain = std::move(bananSwapChain);
            bananSwapChain = std::make_unique<BananSwapChain>(bananDevice, extent, oldSwapChain);
            assert(bananSwapChain->imageCount() == oldSwapChain->imageCount() && "Swap chain image count has changed!");
        }
        createPipeline();
    }

    void BananEngineTest::recordCommandBuffer(int imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to start recording command buffer");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = bananSwapChain->getRenderPass();
        renderPassInfo.framebuffer = bananSwapChain->getFrameBuffer(imageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = bananSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        bananPipeline->bind(commandBuffers[imageIndex]);
        bananModel->bind(commandBuffers[imageIndex]);
        bananModel->draw(commandBuffers[imageIndex]);

        vkCmdEndRenderPass(commandBuffers[imageIndex]);
        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("unable to stop command buffer recording");
        }
    }
}

