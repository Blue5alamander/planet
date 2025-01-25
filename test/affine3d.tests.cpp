#include <planet/affine/matrix3d.hpp>
#include <planet/affine/point3d.hpp>
#include <planet/affine/transform3d.hpp>
#include <felspar/test.hpp>


static_assert(
        sizeof(planet::affine::point3d) == sizeof(float) * 4,
        "There must be zero extra data in the 3d point for Vulkan "
        "compatibility");
static_assert(
        sizeof(planet::affine::matrix3d) == sizeof(float) * 16,
        "There must be zero extra data in the matrix for Vulkan compatibility");


namespace {


    auto const suite = felspar::testsuite("affine3d");


    auto const p3 = suite.test(
            "point3d",
            [](auto check) {
                auto const p = planet::affine::point3d{2, 4, 8, 2};
                check(p.x()) == 1.0f;
                check(p.y()) == 2.0f;
                check(p.z()) == 4.0f;
            },
            [](auto check) {
                auto const p = planet::affine::point3d{2, 4, 8, 2}
                        + planet::affine::point3d{3, 5, 0};
                check(p.x()) == 4.0f;
                check(p.y()) == 7.0f;
                check(p.z()) == 4.0f;
            });


    auto const mat3 = suite.test("matrix", [](auto check) {
        auto const tr = planet::affine::matrix3d::translate(1, 2, 3);
        check(tr[{3, 0}]) == 1.0f;
        check(tr[{3, 1}]) == 2.0f;
        check(tr[{3, 2}]) == 3.0f;

        planet::affine::matrix3d const t2{
                planet::affine::matrix2d::translate({2, 3})};
        check(t2[{3, 0}]) == 2.0f;
        check(t2[{3, 1}]) == 3.0f;
        check(t2[{3, 2}]) == 0.0f;

        auto const t3 = planet::affine::matrix3d::translate(2, 3, 4);
        auto const mul = planet::affine::matrix3d{} * t3;
        check(mul[{0, 0}]) == t3[{0, 0}];
        check(mul[{1, 0}]) == t3[{1, 0}];
        check(mul[{2, 0}]) == t3[{2, 0}];
        check(mul[{3, 0}]) == t3[{3, 0}];
        check(mul[{0, 1}]) == t3[{0, 1}];
        check(mul[{1, 1}]) == t3[{1, 1}];
        check(mul[{2, 1}]) == t3[{2, 1}];
        check(mul[{3, 1}]) == t3[{3, 1}];
        check(mul[{0, 2}]) == t3[{0, 2}];
        check(mul[{1, 2}]) == t3[{1, 2}];
        check(mul[{2, 2}]) == t3[{2, 2}];
        check(mul[{3, 2}]) == t3[{3, 2}];
        check(mul[{0, 3}]) == t3[{0, 3}];
        check(mul[{1, 3}]) == t3[{1, 3}];
        check(mul[{2, 3}]) == t3[{2, 3}];
        check(mul[{3, 3}]) == t3[{3, 3}];

        planet::affine::matrix3d const ry{
                planet::affine::matrix2d::reflect_y()};
        check(ry[{0, 0}]) == 1.0f;
        check(ry[{1, 0}]) == 0.0f;
        check(ry[{2, 0}]) == 0.0f;
        check(ry[{3, 0}]) == 0.0f;
        check(ry[{0, 1}]) == 0.0f;
        check(ry[{1, 1}]) == -1.0f;
        check(ry[{2, 1}]) == 0.0f;
        check(ry[{3, 1}]) == 0.0f;
        check(ry[{0, 2}]) == 0.0f;
        check(ry[{1, 2}]) == 0.0f;
        check(ry[{2, 2}]) == 1.0f;
        check(ry[{3, 2}]) == 0.0f;
        check(ry[{0, 3}]) == 0.0f;
        check(ry[{1, 3}]) == 0.0f;
        check(ry[{2, 3}]) == 0.0f;
        check(ry[{3, 3}]) == 1.0f;
    });


    float constexpr aspect = 1.0f;
    float constexpr theta = 1.0f;
    auto const tran3 = suite.test(
            "transform3d",
            [](auto check, auto &log) {
                auto const t = planet::affine::transform3d::perspective(
                        aspect, theta);
                auto const p = planet::affine::point3d{0, 0, -1};
                auto const r = t.into() * p;
                log << "x " << r.x() << " y " << r.y() << " z " << r.z()
                    << " zh " << r.zh << " h " << r.h << '\n';
                check(r.x()) == 0.0f;
                check(r.y()) == 0.0f;
                check(r.z()) == 1.0f;
            },
            [](auto check, auto &log) {
                auto const t = planet::affine::transform3d::perspective(
                        aspect, theta);
                auto const p = planet::affine::point3d{0, 0, -10};
                auto const r = t.into() * p;
                log << "x " << r.x() << " y " << r.y() << " z " << r.z()
                    << " zh " << r.zh << " h " << r.h << '\n';
                check(r.x()) == 0.0f;
                check(r.y()) == 0.0f;
                check(r.z()) == 0.1f;
            },
            [](auto check, auto &log) {
                auto const t = planet::affine::transform3d::perspective(
                        aspect, theta);
                auto const p = planet::affine::point3d{0, 0, 10};
                auto const r = t.into() * p;
                log << "x " << r.x() << " y " << r.y() << " z " << r.z()
                    << " zh " << r.zh << " h " << r.h << '\n';
                check(r.x()) == 0.0f;
                check(r.y()) == 0.0f;
                check(r.z()) == -0.1f;
            });


}
