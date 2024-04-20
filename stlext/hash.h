//
// Created by yashr on 4/19/24.
//

#include <cstdint>
#include <string>

#pragma once

namespace BananSTLExt {
    template<typename T> class BananHash {
        public:
            uint64_t hash(T const &obj) const noexcept;
    };

    template <typename T> class BananHash<std::basic_string<T>> {
        public:
            uint64_t hash(std::basic_string<T> const &str) const noexcept;
    };

    template <typename T> class BananHash<T*> {
        public:
            uint64_t hash(T* ptr) const noexcept;
    };
}
