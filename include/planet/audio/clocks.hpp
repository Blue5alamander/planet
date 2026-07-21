#pragma once


#include <atomic>
#include <chrono>


namespace planet::audio {


    using namespace std::literals;


    /// ## Standard audio output sample rate
    using sample_clock =
            std::chrono::duration<std::int64_t, std::ratio<1, 48'000>>;

    std::size_t constexpr samples_per_second = sample_clock::period::den;
    std::size_t constexpr nyquist_limit = samples_per_second / 2;


    /// ## The largest buffer slice the audio system will ever produce
    sample_clock constexpr max_buffer_duration{2048};
    /**
     * A compile-time cap that only bounds storage. Every `std::array<float, …>`
     * in the audio path is sized by `max_buffer_samples`, and the working block
     * the mixer and SDL callback actually advance in is always smaller than or
     * equal to this cap, so the arrays always have room for whatever is
     * rendered.
     */
    std::size_t constexpr max_buffer_samples = max_buffer_duration.count();


    /// ## The working buffer size the engine actually advances in
    sample_clock constexpr initial_buffer_duration{512};
    /**
     * The only remaining compile-time working size, and the seed the atomic
     * below starts at. The cap-vs-working-size invariant stays a compile-time
     * check via `max_buffer_duration >= initial_buffer_duration` —
     * `buffer_duration()` reads a `std::atomic` and so is not a constant
     * expression.
     */
    inline std::atomic<sample_clock> active_buffer_duration{
            initial_buffer_duration};
    /**
     * Published as an inline global so the producers below `planet-sdl` — the
     * generators in `planet/`, `b5/src/synth/` and `src/core/`, which cannot
     * see `planet::sdl::configuration` — can read the configured block size.
     * The pattern mirrors `planet::log::active`, which `planet::sdl::load`
     * writes at config-load time. It is written once at config load and read on
     * producer threads, so a relaxed load is sufficient.
     */
    inline sample_clock buffer_duration() noexcept {
        return active_buffer_duration.load(std::memory_order_relaxed);
    }
    inline std::size_t buffer_samples() noexcept {
        return buffer_duration().count();
    }
    /**
     * Producers size their storage by `max_buffer_samples` but fill and yield
     * only the first `buffer_samples()` entries. Yielding whole capacity-sized
     * blocks would also be correct — `raw_mix` re-blocks whatever it is handed
     * — but it would run every generator further ahead of playback than is
     * necessary. This is fine if the generator has no dynamic inputs, but would
     * make dynamic inputs far less responsive than the lower active duration
     * would imply.
     */


}
