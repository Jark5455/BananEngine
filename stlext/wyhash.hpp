//
// Created by yashr on 4/19/24.
//

// Implementation of WyHash Hashing Algorithm

#pragma once

//includes
#include <cstdint>
#include <cstring>

#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
    #pragma intrinsic(_umul128)
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
        static inline uint64_t BananWyHash::wyhash(const void *key, size_t len, uint64_t seed, const uint64_t *secret) {
            const auto *p = (const uint8_t *)key;
            seed ^= wymix(seed ^ secret[0],secret[1]);
            uint64_t a;
            uint64_t b;

            if (len <= 16) [[likely]] {
                if (len >= 4) [[likely]] {
                    a = (wyr4(p) << 32) | wyr4(p + ((len >> 3) << 2));
                    b = (wyr4(p + len - 4) << 32) | wyr4(p + len - 4 - ((len >> 3) << 2));
                } else if (len > 0) [[likely]] {
                    a = _wyr3(p,len);
                    b = 0;
                } else a = b = 0;
            }

            else {
                size_t i = len;
                if (i >= 48) [[unlikely]] {
                    uint64_t see1 = seed, see2 = seed;
                    do [[likely]] {
                        seed = wymix(wyr8(p) ^ secret[1],wyr8(p + 8) ^ seed);
                        see1 = wymix(wyr8(p + 16) ^ secret[2],wyr8(p + 24) ^ see1);
                        see2 = wymix(wyr8(p + 32) ^ secret[3],wyr8(p + 40) ^ see2);
                        p += 48;
                        i -= 48;
                    } while (i >= 48);
                    seed ^= see1 ^ see2;
                }

                while (i > 16) [[unlikely]] {
                    seed = wymix(wyr8(p) ^ secret[1],wyr8(p + 8) ^ seed);
                    i -= 16;
                    p += 16;
                }

                a = wyr8(p + i - 16);
                b = wyr8(p + i - 8);
            }

            a ^= secret[1];
            b ^= seed;
            wymum(&a, &b);

            return wymix(a ^ secret[0] ^ len,b ^ secret[1]);
        }

        static inline uint64_t wyhash64(uint64_t A, uint64_t B) {
            A ^= 0x2d358dccaa6c78a5ull;
            B ^= 0x8bb84b93962eacc9ull;
            wymum(&A, &B);

            return wymix(A ^ 0x2d358dccaa6c78a5ull,B ^ 0x8bb84b93962eacc9ull);
        }

        static inline uint64_t wyrand(uint64_t *seed) {
            *seed += 0x2d358dccaa6c78a5ull;

            return wymix(*seed,*seed ^ 0x8bb84b93962eacc9ull);
        }

        static inline double wy2u01(uint64_t r) {
            const double wynorm = 1.0 / (1ull << 52);
            return static_cast<double>(r >> 12) * wynorm;
        }

        static inline double wy2gau(uint64_t r) {
            const double wynorm = 1.0 / (1ull << 20);
            return static_cast<double>((r & 0x1fffff) + ((r >> 21) & 0x1fffff) + ((r >> 42) & 0x1fffff)) * wynorm - 3.0;
        }

        constexpr static const uint64_t wyp[4] = {0x2d358dccaa6c78a5ull, 0x8bb84b93962eacc9ull, 0x4b33a62ed433d4a3ull, 0x4d5a2da51de1aa47ull};

        static inline uint64_t wyrot(uint64_t x) {
            return (x >> 32) | (x << 32);
        }

        static inline void wymum(uint64_t *A, uint64_t *B) {
            #if defined(__SIZEOF_INT128__)
                __uint128_t r = (*A) * (*B);
                *A ^= (uint64_t)r;
                *B ^= (uint64_t)(r >> 64);
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

        static inline uint64_t wymix(uint64_t A, uint64_t B) {
            wymum(&A, &B);
            return A ^ B;
        }

        static inline uint64_t wyr8(const uint8_t *p) {
            uint64_t v;
            memcpy(&v, p, 8);

            #if WYHASH_LITTLE_ENDIAN
                return v;
            #elif defined(_MSC_VER)
                return _byteswap_uint64(v);
            #else
                return __builtin_bswap64(v);
            #endif
        }

        static inline uint64_t wyr4(const uint8_t *p) {
            uint32_t v;
            memcpy(&v, p, 4);

            #if WYHASH_LITTLE_ENDIAN
                return v;
            #elif defined(_MSC_VER)
                return _byteswap_ulong(v);
            #else
                return __builtin_bswap32(v);
            #endif
        }

        static inline uint64_t _wyr3(const uint8_t *p, size_t k) {
            return (((uint64_t)p[0]) << 16) | (((uint64_t)p[k >> 1]) << 8) | p[k - 1];
        }
    };
}
