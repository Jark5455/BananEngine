//
// Created by yashr on 12/4/21.
//

#include "BananEngineTest.h"

#include "../banan_logger.h"

#include <stdexcept>

namespace Banan{

    BananEngineTest::BananEngineTest() {
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
    }

    BananEngineTest::~BananEngineTest() {
        vkDestroyPipelineLayout(bananDevice.device(), pipelineLayout, nullptr);
    }

    void BananEngineTest::run() {
        BananLogger::initLogger(boost::none);

        while(!bananWindow.windowShouldClose())
        {
            glfwPollEvents();
            drawFrame();
        }
    }

    void BananEngineTest::createPipelineLayout() {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout");
        }
    }

    void BananEngineTest::createPipeline() {
        // why not use the existing variables HEIGHT or WIDTH? - On high pixel density displays such as apples "retina" display these values are incorrect, but the swap chain corrects these values
        auto pipelineConfig = BananPipeline::defaultPipelineConfigInfo(bananSwapChain.width(), bananSwapChain.height());
        pipelineConfig.renderPass = bananSwapChain.getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        bananPipeline = std::make_unique<BananPipeline>(bananDevice, "shaders/triangle.vert.spv", "shaders/triangle.frag.spv", pipelineConfig);
    }

    void BananEngineTest::createCommandBuffers() {
        commandBuffers.resize(bananSwapChain.imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = bananDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(bananDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers");
        }

        for (int i = 0; i < commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to start recording command buffer");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = bananSwapChain.getRenderPass();
            renderPassInfo.framebuffer = bananSwapChain.getFrameBuffer(i);

            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = bananSwapChain.getSwapChainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            bananPipeline->bind(commandBuffers[i]);
            vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

            vkCmdEndRenderPass(commandBuffers[i]);
            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("unable to stop command buffer recording");
            }
        }
    }

    void BananEngineTest::drawFrame() {
        uint32_t imageIndex;
        auto result = bananSwapChain.acquireNextImage(&imageIndex);

        if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image");
        }

        result = bananSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }
}

