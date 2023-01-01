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
                check(ulps(i.x(), -5.0f)) < 2u;
                check(ulps(i.y(), 4.0f)) < 2u;

                auto const o = t.outof({-5, 4});
                check(ulps(o.x(), 4)) < 2u;
                check(ulps(o.y(), 5)) < 2u;
            },
            [](auto check) {
                planet::affine::transform t;
                t.reflect_y().scale(0.5f).translate({3, 7});

                auto const i = t.into({4, 5});
                check(ulps(i.x(), 5)) < 2u;
                check(ulps(i.y(), 4.5f)) < 2u;

                auto const o = t.outof({5, 4.5f});
                check(ulps(o.x(), 4)) < 2u;
                check(ulps(o.y(), 5)) < 2u;
            });


    auto const theta = felspar::testsuite("affine2d/point2d/theta", [](auto check) {
        check(planet::affine::point2d::from_polar(10, 0).x()) == 10.0f;
        check(planet::affine::point2d::from_polar(10, 0).y()) == 0.0f;
        check(planet::affine::point2d::from_polar(10, 0).theta()) == 0.0f;

        check(std::fabs(
                planet::affine::point2d::from_polar(10, 0.125f).x()
                - 10.0f * std::sqrt(2) / 2.0f))
                < 1.0e-5f;
        check(std::fabs(
                planet::affine::point2d::from_polar(10, 0.125f).y()
                - 10.0f * std::sqrt(2) / 2.0f))
                < 1.0e-5f;
        check(planet::affine::point2d::from_polar(10, 0.125).theta()) == 0.125f;

        check(std::fabs(planet::affine::point2d::from_polar(10, 0.25f).x()))
                < 1.0e-5f;
        check(planet::affine::point2d::from_polar(10, 0.25f).y()) == 10.0f;
        check(planet::affine::point2d::from_polar(10, 0.25f).theta()) == 0.25f;

        check(std::fabs(
                planet::affine::point2d::from_polar(10, 0.375f).x()
                + 10.0f * std::sqrt(2) / 2.0f))
                < 1.0e-5f;
        check(std::fabs(
                planet::affine::point2d::from_polar(10, 0.375f).y()
                - 10.0f * std::sqrt(2) / 2.0f))
                < 1.0e-5f;
        check(planet::affine::point2d::from_polar(10, 0.375f).theta())
                == 0.375f;

        check(planet::affine::point2d::from_polar(10, 0.5f).x()) == -10.0f;
        check(std::fabs(planet::affine::point2d::from_polar(10, 0.5f).y()))
                < 1.0e-5f;
        check(planet::affine::point2d::from_polar(10, 0.5f).theta()) == 0.5f;

        check(std::fabs(planet::affine::point2d::from_polar(10, 0.75f).x()))
                < 1.0e-5f;
        check(planet::affine::point2d::from_polar(10, 0.75f).y()) == -10.0f;
        check(planet::affine::point2d::from_polar(10, 0.75f).theta()) == 0.75f;
    });


    auto const rotate =
            felspar::testsuite("affine2d/point2d/rotate", [](auto check) {
                planet::affine::point2d p{10, 0};
                check(p.x()) == 10.0f;
                check(p.y()) == 0.0f;
                check(p.theta()) == 0.0f;

                p = p.rotate(0.125f);
                check(std::fabs(p.x() - 10.0f * std::sqrt(2) / 2.0f)) < 1.0e-5f;
                check(std::fabs(p.y() - 10.0f * std::sqrt(2) / 2.0f)) < 1.0e-5f;
                check(p.theta()) == 0.125f;

                p = p.rotate(0.125f);
                check(std::fabs(p.x())) < 1.0e-5f;
                check(p.y()) == 10.0f;
                check(p.theta()) == 0.25f;

                p = p.rotate(0.125f);
                check(std::fabs(p.x() + 10.0f * std::sqrt(2) / 2.0f)) < 1.0e-5f;
                check(std::fabs(p.y() - 10.0f * std::sqrt(2) / 2.0f)) < 1.0e-5f;
                check(p.theta()) == 0.375f;

                p = p.rotate(0.125f);
                check(p.x()) == -10.0f;
                check(std::fabs(p.y())) < 1.0e-5f;
                check(p.theta()) == 0.5f;

                p = p.rotate(0.25f);
                check(std::fabs(p.x())) < 1.0e-5f;
                check(p.y()) == -10.0f;
                check(p.theta()) == 0.75f;

                p = p.rotate(0.25f);
                check(p.x()) == 10.0f;
                check(std::fabs(p.y())) < 1.0e-5f;
                check(std::fabs(p.theta())) < 1.0e07f;
            });


}
