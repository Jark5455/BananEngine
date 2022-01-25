//
// Created by yashr on 12/4/21.
//

#include "BananEngineTest.h"

#include <cstdlib>
#include <iostream>

int main() {
    Banan::BananEngineTest app{};
    auto logger = app.getLogger();

    try {
        app.run();
    } catch (const std::exception &e) {
        logger->write(Banan::LogLevel::FATAL, e.what());
        logger->flush();
        return EXIT_FAILURE;
    }

    logger->write(Banan::LogLevel::INFO, "Banan exited successfully");
    logger->flush();
    return EXIT_SUCCESS;
}