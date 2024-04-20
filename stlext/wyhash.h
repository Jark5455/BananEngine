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

namespace BananSTLExt {
    class BananWyHash {
        static inline uint64_t wyrot(uint64_t x);
        static inline void wymum(uint64_t *A, uint64_t *B);
        static inline uint64_t wymix(uint64_t A, uint64_t B);
    };
}
