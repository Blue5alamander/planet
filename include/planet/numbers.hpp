#pragma once


#if __has_include(<numbers>)
#include <numbers>
namespace planet::affine {
    /// Constant for radian conversions
    constexpr float τ = std::numbers::pi_v<float> * 2.0f;
}
#else
namespace planet::affine {
    /// Constant for radian conversions
    constexpr float τ = 6.283185307179586f;
}
#endif
