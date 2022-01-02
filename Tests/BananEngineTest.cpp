//
// Created by yashr on 12/4/21.
//

#include "BananEngineTest.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>

namespace Banan{

    struct SimplePushConstantData {
        glm::mat2 transform{1.f};
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };

    BananEngineTest::BananEngineTest() {
        loadGameObjects();
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

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(bananDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void BananEngineTest::createPipeline() {
        // why not use the existing variables HEIGHT or WIDTH? - On high pixel density displays such as apples "retina" display these values are incorrect, but the swap chain corrects these values
        PipelineConfigInfo pipelineConfig{};
        BananPipeline::defaultPipelineConfigInfo(pipelineConfig);
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

    void BananEngineTest::freeCommandBuffers() {
        vkFreeCommandBuffers(bananDevice.device(), bananDevice.getCommandPool(), commandBuffers.size(), commandBuffers.data());
        commandBuffers.clear();
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

            if (bananSwapChain->imageCount() != commandBuffers.size()) {
                freeCommandBuffers();
                createCommandBuffers();
            }

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

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(bananSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(bananSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, bananSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

        renderGameObjects(commandBuffers[imageIndex]);

        vkCmdEndRenderPass(commandBuffers[imageIndex]);
        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("unable to stop command buffer recording");
        }
    }

    void BananEngineTest::loadGameObjects() {
        std::vector<BananModel::Vertex> vertices{
                {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };

        auto bananModel = std::make_shared<BananModel>(bananDevice, vertices);
        auto triangle = BananGameObject::createGameObject();
        triangle.model = bananModel;
        triangle.color = {.1f, .8f, .1f};
        triangle.transform2D.translation.x = .2f;
        triangle.transform2D.scale = {2.f, 0.5f};
        triangle.transform2D.rotation = .25f * glm::two_pi<float>();

        gameObjects.push_back(std::move(triangle));
    }

    void BananEngineTest::renderGameObjects(VkCommandBuffer commandBuffer) {
        bananPipeline->bind(commandBuffer);

        for (auto &obj : gameObjects) {
            obj.transform2D.rotation = glm::mod(obj.transform2D.rotation + 0.01f, glm::two_pi<float>());

            SimplePushConstantData push{};
            push.offset = obj.transform2D.translation;
            push.color = obj.color;
            push.transform = obj.transform2D.mat2();

            vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
        }
    }
}

