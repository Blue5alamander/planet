#include <planet/affine/matrix3d.hpp>
#include <felspar/test.hpp>


namespace {


    auto const mat3 = felspar::testsuite("affine3d/matrix", [](auto check) {
        auto const tr = planet::affine::matrix3d::translate(1, 2, 3);
        check(tr[{3, 0}]) == 1.0f;
        check(tr[{3, 1}]) == 2.0f;
        check(tr[{3, 2}]) == 3.0f;

        planet::affine::matrix3d const t2{
                planet::affine::matrix2d::translate({2, 3})};
        check(t2[{3, 0}]) == 2.0f;
        check(t2[{3, 1}]) == 3.0f;
        check(t2[{3, 2}]) == 0.0f;
    });


}
