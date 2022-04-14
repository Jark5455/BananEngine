//
// Created by yashr on 12/28/21.
//

#include "banan_logger.h"

#include <iostream>
#include <chrono>

namespace Banan {
    BananLogger::BananLogger(const char *filepath) {
        file = nullptr;

        if (filepath != nullptr) {
            file = std::make_unique<std::ofstream>();
            file->open(filepath);
        }
    }

    BananLogger::~BananLogger() {
        if (file != nullptr) file->close();
    }

    void BananLogger::write(LogLevel level, const std::string &message) {
        switch (level) {
            case INFO:
                writeToConsole("[INFO]: " + unixMsTs() + ": " + message);
                break;
            case WARN:
                writeToConsole("[WARN]: " + unixMsTs() + ": " + message);
                break;
            case ERROR:
                writeToConsole("[ERROR]: " + unixMsTs() + ": " + message);
                break;
            case FATAL:
                writeToConsole("[FATAL]: " + unixMsTs() + ": " + message);
                break;
        }
    }

    void BananLogger::writeToConsole(const std::string &message) {
        std::cout << message << '\n';
        if (file != nullptr) writeToFile(message);
    }

    void BananLogger::writeToFile(const std::string &message) {
        file->write(message.c_str(), static_cast<std::streamsize>(message.size()));
    }

    void BananLogger::flush() {
        std::cout << std::endl;
    }

    std::string BananLogger::unixMsTs() {
        return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
    }
}
