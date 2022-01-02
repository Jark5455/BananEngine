//
// Created by yashr on 12/4/21.
//

#define VMA_IMPLEMENTATION
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#include "../banan_logger.h"
#include "BananEngineTest.h"

#include <cstdlib>
#include <iostream>

int main() {
    Banan::BananLogger::initLogger(boost::none);
    Banan::BananEngineTest app{};

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}