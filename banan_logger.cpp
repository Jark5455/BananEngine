//
// Created by yashr on 12/28/21.
//

#include "banan_logger.h"

#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/utility/setup/file.hpp>

#ifdef __unix__
const bool nix = true;

#include <unistd.h>
#include <pwd.h>

#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
const bool nix = false;
#endif

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(banan_logger, boost::log::sources::logger_mt)

namespace Banan{
    void BananLogger::initLogger(boost::optional<std::string> logFilePath) {
        if (logFilePath) {
            boost::log::add_file_log(*logFilePath);
        } else {
            if (nix) {
                char *logfile;
                if ((logfile = getenv("HOME")) == nullptr) {
                    logfile = getpwuid(getuid())->pw_dir;
                }

                std::strcat(logfile, "banan/banan.log");
                boost::log::add_file_log(logfile);
            } else {
                char *logfile = getenv("APPDATA");
                std::strcat(logfile, "Local\\banan\\banan.log");
                boost::log::add_file_log(logfile);
            }
        }
    }

    void BananLogger::write(const std::string& s) {
        if (initialized) {
            boost::log::sources::logger_mt& lg = banan_logger::get();
            boost::log::record rec = lg.open_record();

            if (rec)
            {
                boost::log::record_ostream strm(rec);
                strm << s.c_str();
                strm.flush();
                lg.push_record(boost::move(rec));
            }
        } else {
            initLogger(boost::none);
            write(s);
        }
    }
}
