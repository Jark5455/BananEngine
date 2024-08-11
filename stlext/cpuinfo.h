//
// Created by yashr on 5/15/24.
//

#pragma once

#include <cstdint>

namespace BananSTLExt {
    class CPUInfo {
        public:
            struct VendorID {
                char vendor_id[13];
            };

            static const CPUInfo& getCPUInfo();

            const VendorID VENDOR_ID;
            const uint32_t CACHE_LINE_SIZE;

        private:
            CPUInfo();

            VendorID getVendorID() const;
            uint32_t getCacheLineSize() const;
    };
}
