//
// Created by yashr on 4/19/24.
//

#include <functional>
#include <string>

#include "hash.h"
#include "wyhash.h"

namespace BananSTLExt {
    template<typename T>
    uint64_t BananHash<T>::hash(const T &obj) const noexcept {
        return BananWyHash::wyhash64(static_cast<uint64_t>(obj), UINT64_C(0x9E3779B97F4A7C15));
    }

    template<typename T>
    uint64_t BananHash<T *>::hash(T *ptr) const noexcept {
        return BananWyHash::wyhash64(reinterpret_cast<uintptr_t>(ptr), UINT64_C(0x9E3779B97F4A7C15));
    }

    template<typename T>
    uint64_t BananHash<std::basic_string<T>>::hash(std::basic_string<T> const &str) const noexcept {
        return BananWyHash::wyhash(str.data(), str.size(), BananWyHash::wyp[0], BananWyHash::wyp);
    }
}