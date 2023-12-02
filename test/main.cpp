//
// Created by yashr on 12/4/21.
//

#include "BananEngineTest.h"

#include <cstdlib>
#include <iostream>

int main(int argv, char** args) {
    Banan::BananEngineTest app{};

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cout << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
