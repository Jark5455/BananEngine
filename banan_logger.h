//
// Created by yashr on 12/28/21.
//

#pragma once

#include <memory>
#include <boost/optional.hpp>

namespace Banan {
    class BananLogger {
    public:
        BananLogger(BananLogger const&) = delete;
        BananLogger& operator=(BananLogger const&) = delete;

        static std::shared_ptr<BananLogger> instance()
        {
            static std::shared_ptr<BananLogger> s{new BananLogger};
            return s;
        }

        static void initLogger(boost::optional<std::string> logFilePath);
        void write(const std::string& s);

        bool initialized = false;

    private:
        BananLogger() {}
    };
}