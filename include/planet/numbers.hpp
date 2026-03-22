#pragma once


#if __has_include(<numbers>)
#include <numbers>
namespace planet {
    /// Constant for radian conversions
    float constexpr pi = std::numbers::pi_v<float>;
    float constexpr tau = std::numbers::pi_v<float> * 2.0f;
    float constexpr sqrt3 = std::numbers::sqrt3_v<float>;
    float constexpr phi = std::numbers::phi_v<float>;
}
#else
namespace planet {
    /// Constant for radian conversions
    float constexpr tau = 6.283185307179586f;
    float constexpr pi = tau / 2.0f;
    float constexpr sqrt3 = 1.7320508075688772f;
    float constexpr phi = 1.6180339887498948f;
}
#endif


namespace planet {
    float constexpr log10 = 2.302585092994046f;
}
