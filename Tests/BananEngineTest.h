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
#include <banan_logger.h>

#include <memory>
#include <vector>

int main(int argv, char** args);

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
        std::shared_ptr<BananLogger> getLogger();
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
        std::unique_ptr<BananDescriptorPool> edgeDetectionPool;
        std::unique_ptr<BananDescriptorPool> blendWeightPool;
        std::unique_ptr<BananDescriptorPool> resolvePool;

        std::shared_ptr<BananLogger> bananLogger;
        BananGameObject::Map gameObjects;

        std::unordered_map<uint32_t, VkDescriptorImageInfo> gameObjectsTextureInfo;
        std::unordered_map<uint32_t, VkDescriptorImageInfo> gameObjectsNormalInfo;
        std::unordered_map<uint32_t, VkDescriptorImageInfo> gameObjectsHeightInfo;

        std::unordered_map<uint32_t, glm::mat4> gameObjectModelMatrices;

        std::unique_ptr<BananImage> areaTex;
        std::unique_ptr<BananImage> searchTex;
    };
}
