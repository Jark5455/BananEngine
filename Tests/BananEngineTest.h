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
        static constexpr size_t WIDTH = 800;
        static constexpr size_t HEIGHT = 600;

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

        std::unique_ptr<BananGameObjectManager> bananGameObjectManager;

        std::shared_ptr<BananDescriptorPool> globalPool;
        std::shared_ptr<BananDescriptorPool> globalImagePool;

        std::shared_ptr<BananLogger> bananLogger;

        std::unique_ptr<BananImage> areaTex;
        std::unique_ptr<BananImage> searchTex;
    };
}
