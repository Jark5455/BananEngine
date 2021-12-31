//
// Created by yashr on 12/4/21.
//

#define VMA_IMPLEMENTATION
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#include "BananEngineTest.h"

#include <cstdlib>
#include <iostream>

int main() {
    Banan::BananEngineTest app{};

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}