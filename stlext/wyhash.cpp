//
// Created by yashr on 4/19/24.
//

#include "wyhash.h"

namespace BananSTLExt {
    uint64_t BananWyHash::wyrot(uint64_t x) {
        return (x>>32)|(x<<32);
    }

    void BananWyHash::wymum(uint64_t *A, uint64_t *B) {
        #if defined(__SIZEOF_INT128__)
            __uint128_t r = (*A) * (*B);
            *A ^= (uint64_t)r;
            *B ^= (uint64_t)(r>>64);
        #elif defined(_MSC_VER) && defined(_M_X64)
            uint64_t a;
            uint64_t b;
            a = _umul128(*A, *B, &b);
            *A ^= a;
            *B ^= b;
        #else
            uint64_t ha = *A >> 32;
            uint64_t hb = *B >> 32;
            uint64_t la = (uint32_t)*A;
            uint64_t lb = (uint32_t)*B;
            uint64_t hi
            uint64_t lo;

            uint64_t rh = ha * hb;
            uint64_t rm0 = ha * lb;
            uint64_t rm1 = hb * la;
            uint64_t rl = la * lb;
            uint64_t t = rl + (rm0 << 32);
            uint64_t c = t < rl;

            lo = t + (rm1 << 32);
            c += lo < t;
            hi = rh + (rm0 >> 32) + (rm1 >> 32) + c;
            *A ^= lo;
            *B ^= hi;
        #endif
    }

    uint64_t BananWyHash::wymix(uint64_t A, uint64_t B) {
        wymum(&A,&B);
        return A^B;
    }
}
