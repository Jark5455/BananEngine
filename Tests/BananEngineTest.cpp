//
// Created by yashr on 12/4/21.
//

#include "BananEngineTest.h"
#include "SimpleRenderSystem.h"
#include "KeyboardMovementController.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <chrono>
#include <stdexcept>

namespace Banan{

    struct GlobalUbo {
        glm::mat4 projectionView{1.f};
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, 0.2f};
        glm::vec3 lightPosition{-1.f};
        alignas(16) glm::vec4 lightColor{1.f};
    };

    BananEngineTest::BananEngineTest() {
        globalPool = BananDescriptorPool::Builder(bananDevice).setMaxSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT).addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, BananSwapChain::MAX_FRAMES_IN_FLIGHT).build();
        bananLogger =  std::make_shared<BananLogger>(nullptr);
        loadGameObjects();
    }

    BananEngineTest::~BananEngineTest() = default;

    void BananEngineTest::run() {

        std::vector<std::unique_ptr<BananBuffer>> uboBuffers(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (auto & uboBuffer : uboBuffers) {
            uboBuffer = std::make_unique<BananBuffer>(bananDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffer->map();
        }

        auto globalSetLayout = BananDescriptorSetLayout::Builder(bananDevice).addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS).build();
        std::vector<VkDescriptorSet> globalDescriptorSets(BananSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            BananDescriptorWriter(*globalSetLayout, *globalPool).writeBuffer(0, &bufferInfo).build(globalDescriptorSets[i]);
        }

        SimpleRenderSystem renderSystem{bananDevice, bananRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        BananCamera camera{};

        auto viewerObject = BananGameObject::createGameObject();
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while(!bananWindow.windowShouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(bananWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = bananRenderer.getAspectRatio();
            camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
            camera.setPerspectiveProjection(glm::radians(90.f), aspect, 0.1f, 10.f);

            if (auto commandBuffer = bananRenderer.beginFrame()) {
                int frameIndex = bananRenderer.getFrameIndex();
                BananFrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects};

                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                bananRenderer.beginSwapChainRenderPass(commandBuffer);
                renderSystem.renderGameObjects(frameInfo);
                bananRenderer.endSwapChainRenderPass(commandBuffer);
                bananRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(bananDevice.device());
    }

    void BananEngineTest::loadGameObjects() {
        std::shared_ptr<BananModel> vaseModel = BananModel::createModelFromFile(bananDevice, "banan_assets/ceramic_vase_01_4k.blend");
        auto vase = BananGameObject::createGameObject();
        vase.model = vaseModel;
        vase.transform.translation = {-.5f, .5f, 2.5f};
        vase.transform.rotation = {glm::pi<float>() / 2.0f, 0.0f, 0.0f};
        vase.transform.scale = {3.f, 3.f, 3.f};
        gameObjects.emplace(vase.getId(), std::move(vase));

        std::shared_ptr<BananModel> floorModel = BananModel::createModelFromFile(bananDevice, "banan_assets/quad.obj");
        auto floor = BananGameObject::createGameObject();
        floor.model = floorModel;
        floor.transform.translation = {.5f, .5f, 2.5f};
        floor.transform.rotation = {0.f, 0.f, 0.f};
        floor.transform.scale = {3.f, 1.5f, 3.f};
        gameObjects.emplace(floor.getId(), std::move(floor));
    }

    std::shared_ptr<BananLogger> BananEngineTest::getLogger() {
        return bananLogger;
    }
}

