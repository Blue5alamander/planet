#pragma once


#if __has_include(<numbers>)
#include <numbers>
namespace planet {
    /// Constant for radian conversions
    constexpr float pi = std::numbers::pi_v<float>;
    constexpr float tau = std::numbers::pi_v<float> * 2.0f;
    constexpr float sqrt3 = std::numbers::sqrt3_v<float>;
}
#else
namespace planet {
    /// Constant for radian conversions
    constexpr float tau = 6.283185307179586f;
    constexpr float pi = tau / 2.0f;
    constexpr float sqrt3 = 1.7320508075688772f;
}
#endif
