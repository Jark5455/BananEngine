//
// Created by yashr on 12/4/21.
//

#include "BananEngineTest.h"
#include "../banan_logger.h"

#include <cstdlib>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

int main() {
    Banan::BananEngineTest app{};

    try {
        app.run();
    } catch (const std::exception &e) {
        Banan::LogFatal() << e.what() << "\n";
        return EXIT_FAILURE;
    }

    Banan::LogInfo() << "Banan exited sucessfully";
    return EXIT_SUCCESS;
}