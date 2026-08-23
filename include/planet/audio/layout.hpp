#pragma once


#include <cstddef>


namespace planet::audio {


    /// ## Interleaved/planar stereo layout conversion
    /**
     * The one tested source that both interleaved backends (SDL, WASAPI) and
     * planar backends (JACK, CoreAudio) render from. `frames` counts stereo
     * frames, not individual samples.
     */

    /// ### Split an interleaved `{l, r, ...}` block into planar channels
    void deinterleave(
            float const *interleaved,
            float *left,
            float *right,
            std::size_t frames) noexcept;

    /// ### Merge planar channels into an interleaved `{l, r, ...}` block
    void interleave(
            float const *left,
            float const *right,
            float *interleaved,
            std::size_t frames) noexcept;


}
