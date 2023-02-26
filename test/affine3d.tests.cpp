#include <planet/affine/matrix3d.hpp>
#include <felspar/test.hpp>


namespace {


    auto const mat3 = felspar::testsuite("affine3d/matrix", [](auto check) {
        auto const tr = planet::affine::matrix3d::translate(1, 2, 3);
        check(tr[{3, 0}]) == 1.0f;
        check(tr[{3, 1}]) == 2.0f;
        check(tr[{3, 2}]) == 3.0f;
    });


}
