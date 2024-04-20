//
// Created by yashr on 4/19/24.
//

// Implementation of WyHash Hashing Algorithm

#pragma once

//includes
#include <stdint.h>
#include <string.h>

#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
    #pragma intrinsic(_umul128)
#endif

//likely and unlikely macros
#if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
    #define _likely_(x)  __builtin_expect(x,1)
    #define _unlikely_(x)  __builtin_expect(x,0)
#else
    #define _likely_(x) (x)
    #define _unlikely_(x) (x)
#endif

#if defined(_WIN32) || defined(__LITTLE_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    #define WYHASH_LITTLE_ENDIAN 1
#elif defined(__BIG_ENDIAN__) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    #define WYHASH_LITTLE_ENDIAN 0
#else
    #warning could not determine endianness! Falling back to little endian.
    #define WYHASH_LITTLE_ENDIAN 1
#endif

namespace BananSTLExt {
    class BananWyHash {
        public:
            static inline uint64_t wyhash(const void *key, size_t len, uint64_t seed, const uint64_t *secret);
            static inline uint64_t wyhash64(uint64_t A, uint64_t B);
            static inline uint64_t wyrand(uint64_t *seed);
            static inline double wy2u01(uint64_t r);
            static inline double wy2gau(uint64_t r);

            constexpr static const uint64_t wyp[4] = {0x2d358dccaa6c78a5ull, 0x8bb84b93962eacc9ull, 0x4b33a62ed433d4a3ull, 0x4d5a2da51de1aa47ull};

        private:
            static inline uint64_t wyrot(uint64_t x);
            static inline void wymum(uint64_t *A, uint64_t *B);
            static inline uint64_t wymix(uint64_t A, uint64_t B);

            static inline uint64_t wyr8(const uint8_t *p);
            static inline uint64_t wyr4(const uint8_t *p);

            static inline uint64_t _wyr3(const uint8_t *p, size_t k);
    };
}
