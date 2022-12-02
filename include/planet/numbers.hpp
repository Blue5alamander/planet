#pragma once


#if __has_include(<numbers>)
#include <numbers>
namespace planet {
    /// Constant for radian conversions
    constexpr float τ = std::numbers::pi_v<float> * 2.0f;
    constexpr float sqrt3 = std::numbers::sqrt3_v<float>;
}
#else
namespace planet {
    /// Constant for radian conversions
    constexpr float τ = 6.283185307179586f;
    constexpr float sqrt3 = 1.7320508075688772f;
}
#endif
