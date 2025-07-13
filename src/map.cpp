#include <planet/log.hpp>
#include <planet/map/square.hpp>
#include <planet/serialise.hpp>


/// ### `planet::map::coordinates`


void planet::map::save(serialise::save_buffer &ab, coordinates const c) {
    ab.save_box("_p:m:coord", c.x, c.y);
}
void planet::map::load(serialise::load_buffer &lb, coordinates &c) {
    lb.load_box("_p:m:coord", c.x, c.y);
}
namespace {
    auto const sc = planet::log::format("_p:m:coord", [](auto &os, auto &box) {
        std::int32_t c, r;
        box.named("_p:m:coord", c, r);
        os << "square@(" << c << ", " << r << ')';
    });
}


/// ### `planet::hexmap`


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


/// ### `planet::hexmap::coordinates`


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
    cube<planet::hexmap::coordinates::value_type>
            round_qrs(cube<float> const c) {
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
        return {planet::hexmap::coordinates::value_type(q),
                planet::hexmap::coordinates::value_type(r),
                planet::hexmap::coordinates::value_type(s)};
    }
}


planet::hexmap::coordinates planet::hexmap::coordinates::from_position(
        affine::point2d const p, float const r) {
    auto const qrs = xy_to_qrs(p, r);
    auto const cube = round_qrs(qrs);
    return {2 * cube.q + cube.r, cube.r};
}


std::size_t planet::hexmap::best_direction_index(
        coordinates const from, coordinates const to) {
    /**
     * TODO With a bit more of the correct mathsing this should just be a simple
     * closed form calculation instead of running a search...
     */
    auto const angle = (to.centre() - from.centre()).theta();
    for (std::size_t index{}; auto const ref : angles) {
        if (std::abs(ref - angle) <= 1.0f / 12.0f) {
            return index;
        } else {
            ++index;
        }
    }
    return 5uz;
}
auto planet::hexmap::best_direction(coordinates const from, coordinates const to)
        -> coordinates {
    return directions[best_direction_index(from, to)];
}


void planet::hexmap::save(serialise::save_buffer &ab, coordinates const c) {
    ab.save_box("_p:h:coord", c.column(), c.row());
}
void planet::hexmap::load(serialise::load_buffer &lb, coordinates &c) {
    coordinates::value_type col{}, row{};
    lb.load_box("_p:h:coord", col, row);
    c = {col, row};
}
namespace {
    auto const hc = planet::log::format("_p:h:coord", [](auto &os, auto &box) {
        std::int32_t c, r;
        box.named("_p:h:coord", c, r);
        os << "hex@(" << c << ", " << r << ')';
    });
}
