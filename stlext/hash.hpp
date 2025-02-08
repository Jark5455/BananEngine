//
// Created by yashr on 4/19/24.
//

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <bit>

#include "wyhash.hpp"

#pragma once

#define HASH_STATICCAST(T)                                                \
        template <>                                                       \
        struct BananHash<T> {                                             \
            auto operator()(const T& obj) const noexcept -> uint64_t {    \
                return std::hash<uint64_t>(static_cast<uint64_t>(obj));   \
            }                                                             \
        }

#define HASH_BITCAST(T)                                                   \
        template <>                                                       \
        struct BananHash<T> {                                             \
            auto operator()(const T& obj) const noexcept -> uint64_t {    \
                return std::hash<uint64_t>(std::bit_cast<uint64_t>(obj)); \
            }                                                             \
        }

namespace BananSTLExt {

    template<typename T>
    struct BananHash<T *> {
        inline uint64_t operator()(T* ptr) const noexcept {
            return std::hash<uintptr_t>(static_cast<uintptr_t>(ptr));
        }
    };

    template<typename T>
    struct BananHash<std::unique_ptr<T>> {
        inline uint64_t operator()(std::unique_ptr<T> ptr) const noexcept {
            return std::hash<uintptr_t>(static_cast<uintptr_t>(ptr));
        }
    };

    template<typename T>
    struct BananHash<std::shared_ptr<T>> {
        inline uint64_t operator()(std::shared_ptr<T> ptr) const noexcept {
            return std::hash<uintptr_t>(static_cast<uintptr_t>(ptr));
        }
    };

    template<typename T>
    struct BananHash<std::basic_string<T>> {
        inline uint64_t operator()(const std::basic_string<T> str) const noexcept {
            return BananWyHash::wyhash(str.data(), str.size(), BananWyHash::wyp[0], BananWyHash::wyp);
        }
    };

    template<typename T>
    struct BananHash<std::basic_string_view<T>> {
        inline uint64_t operator()(const std::basic_string_view str) const noexcept {
            return BananWyHash::wyhash(str.data(), str.size(), BananWyHash::wyp[0], BananWyHash::wyp);
        }
    };

    template<typename T>
    struct BananHash<std::vector<T>> {
        inline uint64_t operator()(const std::vector<T> vec) const noexcept {
            return BananWyHash::wyhash(vec.data(), vec.size(), BananWyHash::wyp[0], BananWyHash::wyp);
        }
    };

    HASH_STATICCAST(bool);
    HASH_STATICCAST(char);
    HASH_STATICCAST(uint8_t);
    HASH_STATICCAST(int);
    HASH_STATICCAST(uint32_t);
    HASH_BITCAST(float);
    HASH_BITCAST(double);
}
