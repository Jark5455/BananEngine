//
// Created by yashr on 12/28/21.
//

#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>

namespace Banan {
    enum LogLevel {
        INFO = 3,
        WARN = 2,
        ERROR = 1,
        FATAL = 0
    };

    class LogInfo {
    public:
        LogInfo(std::ostream& out = std::cout) : m_Out(out) {}
        ~LogInfo() {
            m_Stream << "\n";
            m_Out << m_Stream.rdbuf();
            m_Out.flush();
        }
        template <class T>
        LogInfo& operator<<(const T& thing) { m_Stream << "[INFO]: " << thing; return *this; }
    private:
        std::stringstream m_Stream;
        std::ostream& m_Out;
    };

    class LogWarn {
    public:
        LogWarn(std::ostream& out = std::cout) : m_Out(out) {}
        ~LogWarn() {
            m_Stream << "\n";
            m_Out << m_Stream.rdbuf();
            m_Out.flush();
        }
        template <class T>
        LogWarn& operator<<(const T& thing) { m_Stream << "[Error]: "<< thing; return *this; }
    private:
        std::stringstream m_Stream;
        std::ostream& m_Out;
    };

    class LogError {
    public:
        LogError(std::ostream& out = std::cout) : m_Out(out) {}
        ~LogError() {
            m_Stream << "\n";
            m_Out << m_Stream.rdbuf();
            m_Out.flush();
        }
        template <class T>
        LogError& operator<<(const T& thing) { m_Stream << "[Error]: "<< thing; return *this; }
    private:
        std::stringstream m_Stream;
        std::ostream& m_Out;
    };

    class LogFatal {
    public:
        LogFatal(std::ostream& out = std::cout) : m_Out(out) {}
        ~LogFatal() {
            m_Stream << "\n";
            m_Out << m_Stream.rdbuf();
            m_Out.flush();
        }
        template <class T>
        LogFatal& operator<<(const T& thing) { m_Stream << "[FATAL]: "<< thing; return *this; }
    private:
        std::stringstream m_Stream;
        std::ostream& m_Out;
    };
}