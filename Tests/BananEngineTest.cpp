//
// Created by yashr on 12/4/21.
//

#include "BananEngineTest.h"

#include "../banan_logger.h"

namespace Banan{
    void Banan::BananEngineTest::run() {
        Banan::BananLogger::initLogger(boost::none);

        while(!bananWindow.windowShouldClose())
        {
            glfwPollEvents();
        }
    }
}

