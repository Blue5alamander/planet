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


    auto const at = felspar::testsuite(
            "padding.ui/add_to",
            [](auto check) {
                constexpr planet::ui::padding p{2, 3, 4, 5};
                check(p.add_to(planet::affine::extents2d{92, 54}))
                        == planet::affine::extents2d{100, 60};
            },
            [](auto check) {
                /// Adding back what was removed gives the rectangle again
                constexpr planet::ui::padding p{2, 3, 4, 5};
                constexpr planet::affine::rectangle2d r{
                        {10, 20}, planet::affine::extents2d{100, 60}};
                check(p.add_to(p.remove_from(r)).top_left) == r.top_left;
                check(p.add_to(p.remove_from(r)).extents) == r.extents;
            });


    constexpr planet::ui::padding::constrained_type space{
            {100, 20, 200}, {60, 10, 120}};

    auto const rl = felspar::testsuite(
            "padding.ui/reflow",
            [](auto check) {
                /// The lambda lays out in the space left by the padding
                constexpr planet::ui::padding p{.top = 2, .right = 3};
                p.reflow(
                        {.screen = space}, space,
                        [&](auto const &, auto const &inner) {
                            check(inner.width.value()) == 94;
                            check(inner.width.min()) == 14;
                            check(inner.width.max()) == 194;
                            check(inner.height.value()) == 56;
                            check(inner.height.min()) == 6;
                            check(inner.height.max()) == 116;
                            return inner;
                        });
            },
            [](auto check) {
                /// A lambda that fills what it is offered is given back `ex`
                constexpr planet::ui::padding p{.top = 2, .right = 3};
                auto const outer = p.reflow(
                        {.screen = space}, space,
                        [](auto const &, auto const &inner) { return inner; });
                check(outer.width.value()) == space.width.value();
                check(outer.width.min()) == space.width.min();
                check(outer.width.max()) == space.width.max();
                check(outer.height.value()) == space.height.value();
                check(outer.height.min()) == space.height.min();
                check(outer.height.max()) == space.height.max();
            },
            [](auto check) {
                /// The content's own minimum comes back out padded
                constexpr planet::ui::padding p{.top = 2, .right = 3};
                auto const outer = p.reflow(
                        {.screen = space}, space,
                        [](auto const &, auto const &) {
                            return planet::ui::padding::constrained_type{
                                    {40, 30, 194}, {25, 20, 116}};
                        });
                check(outer.width.value()) == 46;
                check(outer.width.min()) == 36;
                check(outer.height.value()) == 29;
                check(outer.height.min()) == 24;
            });


    constexpr planet::affine::rectangle2d area{
            {10, 20}, planet::affine::extents2d{100, 60}};

    auto const op = felspar::testsuite(
            "padding.ui/move_to_with_outer_padding",
            [](auto check) {
                /**
                 * The lambda lays out inside the padding, and what it lays out
                 * in is all that is claimed -- the ring is nobody's
                 */
                constexpr planet::ui::padding p{.top = 2, .right = 3};
                auto const used = p.move_to_with_outer_padding(
                        {.screen = space}, area,
                        [&](auto const &, auto const &inner) {
                            check(inner.top_left)
                                    == planet::affine::point2d{13, 22};
                            check(inner.extents)
                                    == planet::affine::extents2d{94, 56};
                            return inner;
                        });
                check(used.top_left) == planet::affine::point2d{13, 22};
                check(used.extents) == planet::affine::extents2d{94, 56};
            },
            [](auto check) {
                /// Content that uses less keeps only what it used
                constexpr planet::ui::padding p{.top = 2, .right = 3};
                auto const used = p.move_to_with_outer_padding(
                        {.screen = space}, area,
                        [](auto const &, auto const &inner) {
                            return planet::affine::rectangle2d{
                                    inner.top_left,
                                    planet::affine::extents2d{10, 10}};
                        });
                check(used.top_left) == planet::affine::point2d{13, 22};
                check(used.extents) == planet::affine::extents2d{10, 10};
            });


    auto const im = felspar::testsuite(
            "padding.ui/move_to_with_inner_margin",
            [](auto check) {
                /**
                 * A lambda that fills what it is offered gets back `r`, the
                 * margin being part of what the element covers
                 */
                constexpr planet::ui::padding p{.top = 2, .right = 3};
                auto const used = p.move_to_with_inner_margin(
                        {.screen = space}, area,
                        [](auto const &, auto const &inner) { return inner; });
                check(used.top_left) == area.top_left;
                check(used.extents) == area.extents;
            },
            [](auto check) {
                /// Content that uses less still covers its margin around it
                constexpr planet::ui::padding p{.top = 2, .right = 3};
                auto const used = p.move_to_with_inner_margin(
                        {.screen = space}, area,
                        [](auto const &, auto const &inner) {
                            return planet::affine::rectangle2d{
                                    inner.top_left,
                                    planet::affine::extents2d{10, 10}};
                        });
                check(used.top_left) == planet::affine::point2d{10, 20};
                check(used.extents) == planet::affine::extents2d{16, 14};
            });


}
