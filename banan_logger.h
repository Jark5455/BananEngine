//
// Created by yashr on 12/28/21.
//

#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <memory>

namespace Banan {
    enum LogLevel {
        INFO = 3,
        WARN = 2,
        ERROR = 1,
        FATAL = 0
    };

    class BananLogger {
        public:
            BananLogger(const char *filepath);
            ~BananLogger();

            void write(LogLevel level, const std::string &message);
            void flush();

        private:
            void writeToConsole(const std::string &message);
            void writeToFile(const std::string &message);

            static std::string unixMsTs();

            std::unique_ptr<std::ofstream> file;

    };
}
