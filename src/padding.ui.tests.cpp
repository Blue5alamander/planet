#include <planet/ostream.hpp>
#include <planet/ui/padding.hpp>
#include <felspar/test.hpp>


namespace {


    /// ## The CSS shorthand rules that the side defaults implement
    constexpr planet::ui::padding every_side{2};
    static_assert(every_side.top == 2);
    static_assert(every_side.right == 2);
    static_assert(every_side.bottom == 2);
    static_assert(every_side.left == 2);

    constexpr planet::ui::padding vertical_horizontal{2, 3};
    static_assert(vertical_horizontal.top == 2);
    static_assert(vertical_horizontal.right == 3);
    static_assert(vertical_horizontal.bottom == 2);
    static_assert(vertical_horizontal.left == 3);

    constexpr planet::ui::padding top_horizontal_bottom{2, 3, 4};
    static_assert(top_horizontal_bottom.top == 2);
    static_assert(top_horizontal_bottom.right == 3);
    static_assert(top_horizontal_bottom.bottom == 4);
    static_assert(top_horizontal_bottom.left == 3);

    constexpr planet::ui::padding each_side{2, 3, 4, 5};
    static_assert(each_side.top == 2);
    static_assert(each_side.right == 3);
    static_assert(each_side.bottom == 4);
    static_assert(each_side.left == 5);

    /// ## The mirroring the named vertical/horizontal call sites rely on
    constexpr planet::ui::padding named{.top = 2, .right = 3};
    static_assert(named.bottom == named.top);
    static_assert(named.left == named.right);


    auto const rm = felspar::testsuite(
            "padding.ui/remove_from",
            [](auto check) {
                constexpr planet::ui::padding p{.top = 2, .right = 3};
                auto const r = p.remove_from(
                        planet::affine::rectangle2d{
                                {10, 20}, planet::affine::extents2d{100, 60}});
                check(r.top_left) == planet::affine::point2d{13, 22};
                check(r.extents) == planet::affine::extents2d{94, 56};
            },
            [](auto check) {
                constexpr planet::ui::padding p{2, 3, 4, 5};
                check(p.remove_from(planet::affine::extents2d{100, 60}))
                        == planet::affine::extents2d{92, 54};
            });


}
