//
// Created by yashr on 12/4/21.
//
#pragma once

#include "../banan_window.h"
#include "../banan_device.h"
#include "../banan_model.h"
#include "../banan_game_object.h"
#include "../banan_renderer.h"
#include "../banan_logger.h"

#include <memory>
#include <vector>

int main();
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

        std::shared_ptr<BananLogger> bananLogger;
        std::vector<BananGameObject> gameObjects;
    };
}
