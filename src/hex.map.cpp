#include <planet/map/hex.hpp>


bool planet::hexmap::is_within(affine::point2d const c, float const ir) {
    /// kx is `-cos(pi/6)` and ky is `sin(pi/6)`. kr describes the outer radius
    constexpr float kx{-0.8660254037844387f}, ky{0.5f};
    /// Our hex is point up, so swap x & y
    float px{std::abs(c.y())}, py{std::abs(c.x())};
    auto const min_dot = std::min(0.0f, kx * px + ky * py);
    py -= 2.0f * min_dot * ky;
    py -= ir;
    return py < 0.0f;
}


/// ## `planet::hexmap::coordinates`


namespace {
    /// https://www.redblobgames.com/grids/hexagons/#rounding
    template<typename F>
    struct cube {
        F q = {}, r = {}, s = {-q - r};
    };
    cube<float> xy_to_qrs(planet::affine::point2d const p, float const ir) {
        auto const x = p.x(), y = p.y();
        auto const oR = 2.0f * ir / planet::sqrt3;
        return {(x * planet::sqrt3 / 3.0f - y / 3.0f) / oR,
                2.0f * y / 3.0f / oR};
    }
    cube<long> round_qrs(cube<float> const c) {
        auto q = std::round(c.q);
        auto r = std::round(c.r);
        auto s = std::round(c.s);
        auto const qd = std::abs(q - c.q);
        auto const rd = std::abs(r - c.r);
        auto const sd = std::abs(s - c.s);
        if (qd > rd and qd > sd) {
            q = -r - s;
        } else if (rd > sd) {
            r = -q - s;
        } else {
            s = -q - r;
        }
        return {long(q), long(r), long(s)};
    }
}


planet::hexmap::coordinates planet::hexmap::coordinates::from_position(
        affine::point2d const p, float const r) {
    auto const qrs = xy_to_qrs(p, r);
    auto const cube = round_qrs(qrs);
    return {2 * cube.q + cube.r, cube.r};
}
