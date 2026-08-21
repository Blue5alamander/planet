#pragma once


namespace planet::threading {


    /// ## Flush floating point denormals to zero on the calling thread
    void flush_denormals_to_zero() noexcept;
    /**
     * Arithmetic that produces denormal (subnormal) values is handled in
     * microcode on most CPUs and runs far slower than normal floating point. A
     * signal decaying towards silence drifts into that range, so an audio
     * thread flushes denormals to zero to hold a steady cost. The rounding
     * error this introduces sits well below the audible floor.
     *
     * The mode lives in a per-thread hardware register, so every thread that
     * does audio work must call this for itself.
     */


}
