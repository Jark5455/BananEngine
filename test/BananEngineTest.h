//
// Created by yashr on 12/4/21.
//
#pragma once

#include <banan_descriptor.h>
#include <banan_window.h>
#include <banan_device.h>
#include <banan_model.h>
#include <banan_game_object.h>
#include <banan_renderer.h>

#include <memory>
#include <vector>

namespace Banan{
    class BananEngineTest {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        BananEngineTest(const BananEngineTest &) = delete;
        BananEngineTest &operator=(const BananEngineTest &) = delete;

        BananEngineTest();
        ~BananEngineTest();

        void run();

    private:
        void loadGameObjects();

        BananWindow bananWindow{WIDTH, HEIGHT};
        BananDevice bananDevice{bananWindow};
        BananRenderer bananRenderer{bananWindow, bananDevice};

        std::unique_ptr<BananDescriptorPool> globalPool;
        std::unique_ptr<BananDescriptorPool> texturePool;
        std::unique_ptr<BananDescriptorPool> normalPool;
        std::unique_ptr<BananDescriptorPool> heightPool;
        std::unique_ptr<BananDescriptorPool> procrastinatedPool;

        BananGameObject::Map gameObjects;

        std::unordered_map<uint32_t, VkDescriptorImageInfo> gameObjectsTextureInfo;
        std::unordered_map<uint32_t, VkDescriptorImageInfo> gameObjectsNormalInfo;
        std::unordered_map<uint32_t, VkDescriptorImageInfo> gameObjectsHeightInfo;

        std::unordered_map<uint32_t, glm::mat4> gameObjectModelMatrices;
    };
}
