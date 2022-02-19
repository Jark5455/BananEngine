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

    BananEngineTest::BananEngineTest() {
        bananLogger =  std::make_shared<BananLogger>(nullptr);
        loadGameObjects();
    }

    BananEngineTest::~BananEngineTest() {}

    void BananEngineTest::run() {

        SimpleRenderSystem renderSystem{bananDevice, bananRenderer.getSwapChainRenderPass()};
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
                bananRenderer.beginSwapChainRenderPass(commandBuffer);
                renderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
                bananRenderer.endSwapChainRenderPass(commandBuffer);
                bananRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(bananDevice.device());
    }

    void BananEngineTest::loadGameObjects() {
        std::shared_ptr<BananModel> bananModel = BananModel::createModelFromFile(bananDevice, "banan_assets/cow-nonormals.obj");
        auto cube = BananGameObject::createGameObject();
        cube.model = bananModel;
        cube.transform.translation = {.0f, .0f, 2.5f};
        cube.transform.rotation = {0.0f, 0.0f, glm::pi<float>()};
        cube.transform.scale = {.5f, .5f, .5f};

        gameObjects.push_back(std::move(cube));
    }

    std::shared_ptr<BananLogger> BananEngineTest::getLogger() {
        return bananLogger;
    }
}

