#include <planet/camera/target3d.hpp>
#include <felspar/test.hpp>


namespace {


    auto const suite = felspar::testsuite("cameras");


    auto const t3d = suite.test("target3dxy", [](auto check) {
        planet::camera::target3dxy camera;
        planet::affine::point3d p{};
        auto const v = camera.view.into(p);
        check(v.x()) == 0.0f;
        check(v.y()) == 0.0f;
        check(v.z()) == -3.0f;
        check(v.h) == 1.0f;
        auto const f = camera.perspective.into() * v;
        check(f.x()) == 0.0f;
        check(f.y()) == 0.0f;
        check(f.z()) == 1.0f / 3.0f;
    });


}
