//
// Created by yashr on 5/12/24.
//

#include "wyhash.h"

namespace BananSTLExt {
    uint64_t BananWyHash::wyhash(const void *key, size_t len, uint64_t seed, const uint64_t *secret) {
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
}