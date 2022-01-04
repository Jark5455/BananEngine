//
// Created by yashr on 12/4/21.
//

#include "BananEngineTest.h"
#include "SimpleRenderSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>

namespace Banan{

    BananEngineTest::BananEngineTest() {
        loadGameObjects();
    }

    BananEngineTest::~BananEngineTest() {}

    void BananEngineTest::run() {

        SimpleRenderSystem renderSystem{bananDevice, bananRenderer.getSwapChainRenderPass()};

        while(!bananWindow.windowShouldClose())
        {
            glfwPollEvents();

            if (auto commandBuffer = bananRenderer.beginFrame()) {
                bananRenderer.beginSwapChainRenderPass(commandBuffer);
                renderSystem.renderGameObjects(commandBuffer, gameObjects);
                bananRenderer.endSwapChainRenderPass(commandBuffer);
                bananRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(bananDevice.device());
    }

    void BananEngineTest::loadGameObjects() {
        std::vector<BananModel::Vertex> vertices{
                //{{-0.433f, -0.25f}, {1.0f, 0.0f, 0.0f}},
                //{{0.433f, -0.25f}, {0.0f, 1.0f, 0.0f}},
                //{{0.0f, 0.5f}, {0.0f, 0.0f, 1.0f}}

                {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };

        auto bananModel = std::make_shared<BananModel>(bananDevice, vertices);
        auto triangle = BananGameObject::createGameObject();
        triangle.model = bananModel;
        triangle.color = {.1f, .8f, .1f};
        triangle.transform2D.translation.x = 0.0f;
        triangle.transform2D.scale = {2.0f, 0.5f};
        triangle.transform2D.rotation = .25f * glm::two_pi<float>();

        gameObjects.push_back(std::move(triangle));
    }
}

