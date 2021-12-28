//
// Created by yashr on 12/4/21.
//
#pragma once

#include "../banan_window.h"

int main();
namespace Banan{
    class BananEngineTest {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        void run();
    private:
        BananWindow bananWindow{};
    };
}
