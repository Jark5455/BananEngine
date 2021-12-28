//
// Created by yashr on 12/4/21.
//

#include "BananEngineTest.h"
namespace Banan{
    void Banan::BananEngineTest::run() {
        while(!bananWindow.windowShouldClose())
        {
            glfwPollEvents();
        }
    }
}

