//
// Created by yashr on 5/15/24.
//

#include <cstring>

#include "cpuinfo.h"

#if (defined(__GNUC__) || defined(__llvm__)) && defined(__i386__)
#define cpuid(func, a, b, c, d)              \
    __asm__ __volatile__(                    \
        "        pushl %%ebx        \n"      \
        "        xorl %%ecx,%%ecx   \n"      \
        "        cpuid              \n"      \
        "        movl %%ebx, %%esi  \n"      \
        "        popl %%ebx         \n"      \
        : "=a"(a), "=S"(b), "=c"(c), "=d"(d) \
        : "a"(func)                          \
    )
#elif (defined(__GNUC__) || defined(__llvm__)) && defined(__x86_64__)
#define cpuid(func, a, b, c, d)              \
    __asm__ __volatile__ (                    \
        "        pushq %%rbx        \n"      \
        "        xorq %%rcx,%%rcx   \n"      \
        "        cpuid              \n"      \
        "        movq %%rbx, %%rsi  \n"      \
        "        popq %%rbx         \n"      \
        : "=a"(a), "=S"(b), "=c"(c), "=d"(d) \
        : "a"(func)                          \
    )
#elif (defined(_MSC_VER) && defined(_M_IX86)) || defined(__WATCOMC__)
#define cpuid(func, a, b, c, d)              \
    __asm {                                  \
        __asm mov eax, func                  \
        __asm xor ecx, ecx                   \
        __asm cpuid                          \
        __asm mov a, eax                     \
        __asm mov b, ebx                     \
        __asm mov c, ecx                     \
        __asm mov d, edx                     \
    }
#elif (defined(_MSC_VER) && defined(_M_X64))
/* Use __cpuidex instead of __cpuid because ICL does not clear ecx register */
#define cpuid(func, a, b, c, d)              \
    {                                        \
        int CPUInfo[4];                      \
        __cpuidex(CPUInfo, func, 0);         \
        a = CPUInfo[0];                      \
        b = CPUInfo[1];                      \
        c = CPUInfo[2];                      \
        d = CPUInfo[3];                      \
    }
#else
#define cpuid(func, a, b, c, d)              \
    do {                                     \
        a = b = c = d = 0;                   \
        (void)a;                             \
        (void)b;                             \
        (void)c;                             \
        (void)d;                             \
    } while (0)
#endif

namespace BananSTLExt {

    CPUInfo const &CPUInfo::getCPUInfo() {
        static CPUInfo INSTANCE;
        return INSTANCE;
    }

    CPUInfo::VendorID CPUInfo::getVendorID() const {

        struct VendorID vendor_id{};
        char *vendor_id_ptr = vendor_id.vendor_id;

        __asm__ __volatile__ (
            "movl $0, %%eax\n\t"
            "cpuid\n\t"
            "movl %%ebx, %0\n\t"
            "movl %%edx, %1\n\t"
            "movl %%ecx, %2\n\t"
            : "=m"(vendor_id_ptr[0]), "=m"(vendor_id_ptr[4]), "=m"(vendor_id_ptr[8])  // outputs
            : // no inputs
            : "%eax", "%ebx", "%edx", "%ecx", "memory" // clobbered registers
        );

        vendor_id_ptr[12] = 0;
        return vendor_id;
    }

    uint32_t CPUInfo::getCacheLineSize() const {
        int a, b, c, d;
        (void)a;
        (void)b;
        (void)c;
        (void)d;

        if (strcmp(VENDOR_ID.vendor_id, "GenuineIntel") == 0) {
            cpuid(0x00000001, a, b, c, d);
            return ((b >> 8) & 0xff) * 8;
        } else if (strcmp(VENDOR_ID.vendor_id, "AuthenticAMD") == 0) {
            cpuid(0x80000005, a, b, c, d);
            return c & 0xff;
        }

        return 64;
    }

    CPUInfo::CPUInfo() : VENDOR_ID(getVendorID()), CACHE_LINE_SIZE(getCacheLineSize()) {}
}
