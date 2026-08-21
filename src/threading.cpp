#include <planet/threading.hpp>

#if defined(__x86_64__) or defined(__i386__) or defined(_M_X64) \
        or defined(_M_IX86)
#include <pmmintrin.h>
#include <xmmintrin.h>
#elif (defined(__aarch64__) or defined(__arm__)) and defined(__GNUC__)
#include <cstdint>
#endif


void planet::threading::flush_denormals_to_zero() noexcept {
#if defined(__x86_64__) or defined(__i386__) or defined(_M_X64) \
        or defined(_M_IX86)
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
#elif defined(__aarch64__) and defined(__GNUC__)
    /// Bit 24 (`FZ`) of `FPCR` flushes both denormal operands and results.
    std::uint64_t fpcr;
    __asm__ __volatile__("mrs %0, fpcr" : "=r"(fpcr));
    __asm__ __volatile__("msr fpcr, %0" ::"r"(fpcr | (std::uint64_t{1} << 24)));
#elif defined(__arm__) and defined(__GNUC__)
    /// Bit 24 (`FZ`) of `FPSCR` is the 32-bit VFP flush-to-zero control.
    std::uint32_t fpscr;
    __asm__ __volatile__("vmrs %0, fpscr" : "=r"(fpscr));
    __asm__ __volatile__(
            "vmsr fpscr, %0" ::"r"(fpscr | (std::uint32_t{1} << 24)));
#else
#error "No denormal flush control for this architecture or compiler; add one here rather than let denormals stall the audio threads"
#endif
}
