#include <planet/ostream.hpp>
#include <felspar/test.hpp>

#include <cstring>


namespace {


    inline std::uint32_t bitpattern(float const x) {
        std::uint32_t u;
        std::memcpy(&u, &x, sizeof(x));
        return u;
    }
    inline std::uint32_t reset_sign(std::uint32_t const x) {
        return std::int32_t(x) >= 0 ? x : -x;
    }
    inline std::uint32_t ulps(float const a, float const b) {
        std::uint32_t ua = bitpattern(a);
        std::uint32_t ub = bitpattern(b);
        std::uint32_t s = ub ^ ua;

        if (std::int32_t(s) >= 0) {
            return reset_sign(ua - ub);
        } else {
            return ua + ub + 0x80000000;
        }
    }


    auto const transform = felspar::testsuite(
            "affine2d/transform",
            [](auto check) {
                planet::affine::transform t;
                t.rotate(0.25f);
                auto const i = t.into({4, 5});
                check(ulps(i.x(), -5.0f)) < 2;
                check(ulps(i.y(), 4.0f)) < 2;

                auto const o = t.outof({-5, 4});
                check(ulps(o.x(), 4)) < 2;
                check(ulps(o.y(), 5)) < 2;
            },
            [](auto check) {
                planet::affine::transform t;
                t.reflect_y().scale(0.5f).translate({3, 7});

                auto const i = t.into({4, 5});
                check(ulps(i.x(), 5)) < 2;
                check(ulps(i.y(), 4.5f)) < 2;

                auto const o = t.outof({5, 4.5f});
                check(ulps(o.x(), 4)) < 2;
                check(ulps(o.y(), 5)) < 2;
            });


}
