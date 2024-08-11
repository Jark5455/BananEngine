//
// Created by yashr on 4/19/24.
//

#include <cstdint>
#include <string>

#include "wyhash.h"

#pragma once

namespace BananSTLExt {
    template<typename T>
    struct BananHash {
        inline uint64_t operator()(const T &obj) const noexcept {
            return BananWyHash::wyhash64(static_cast<uint64_t>(obj), UINT64_C(0x9E3779B97F4A7C15));
        }
    };

    template<typename T>
    struct BananHash<T *> {
        inline uint64_t operator()(T* obj) const noexcept {
            return BananWyHash::wyhash64(reinterpret_cast<uintptr_t>(obj), UINT64_C(0x9E3779B97F4A7C15));
        }
    };

    template<typename T>
    struct BananHash<std::basic_string<T>> {
        inline uint64_t operator()(const std::basic_string<T> str) const noexcept {
            return BananWyHash::wyhash(str.data(), str.size(), BananWyHash::wyp[0], BananWyHash::wyp);
        }
    };
}
