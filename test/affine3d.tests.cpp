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
    });


}
