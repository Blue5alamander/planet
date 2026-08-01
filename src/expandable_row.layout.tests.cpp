#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/layout.expandable_row.hpp>
#include <felspar/test.hpp>


namespace {


    /// ## An element that takes up all of the width it is offered
    struct filling_element final : public planet::ui::reflowable {
        float height;


        filling_element(float const h)
        : reflowable{"filling_element"}, height{h} {}


      private:
        constrained_type do_reflow(
                reflow_parameters const &, constrained_type const &c) override {
            return {c.width.value(), height};
        }
        planet::affine::rectangle2d move_sub_elements(
                reflow_parameters const &,
                planet::affine::rectangle2d const &r) override {
            return r;
        }
    };


    constexpr planet::ui::reflowable::constrained_type screen{
            {100, 0, 100}, {100, 0, 100}};

    constexpr planet::affine::rectangle2d full{
            {15, 20}, planet::affine::extents2d{100, 100}};


    auto const suite = felspar::testsuite("expandable_row.layout");


    auto const declined =
            suite.test("the offer is declined", [](auto check, auto &log) {
                /// The row is then laid out exactly as a `row` would be
                auto c = planet::ui::expandable_row{
                        0,
                        std::tuple{
                                planet::debug::fixed_element{log, {4, 3}},
                                planet::debug::fixed_element{log, {4, 3}}},
                        2.0f};
                c.reflow({.screen = screen}, screen);
                c.move_to({.screen = screen}, full);

                check(c.position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{10, 3}};
                check(std::get<0>(c.items).position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{4, 3}};
                check(std::get<1>(c.items).position())
                        == planet::affine::rectangle2d{
                                {21, 20}, planet::affine::extents2d{4, 3}};
            });


    auto const taken =
            suite.test("the offer is taken", [](auto check, auto &log) {
                auto c = planet::ui::expandable_row{
                        0,
                        std::tuple{
                                filling_element{3},
                                planet::debug::fixed_element{log, {4, 3}}},
                        2.0f};
                c.reflow({.screen = screen}, screen);
                c.move_to({.screen = screen}, full);

                check(c.position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{100, 3}};
                check(std::get<0>(c.items).position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{94, 3}};
                check(std::get<1>(c.items).position())
                        == planet::affine::rectangle2d{
                                {111, 20}, planet::affine::extents2d{4, 3}};
            });


    auto const middle =
            suite.test("expanding in the middle", [](auto check, auto &log) {
                auto c = planet::ui::expandable_row{
                        1,
                        std::tuple{
                                planet::debug::fixed_element{log, {4, 3}},
                                filling_element{3},
                                planet::debug::fixed_element{log, {4, 3}}},
                        2.0f};
                c.reflow({.screen = screen}, screen);
                c.move_to({.screen = screen}, full);

                check(c.position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{100, 3}};
                check(std::get<0>(c.items).position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{4, 3}};
                check(std::get<1>(c.items).position())
                        == planet::affine::rectangle2d{
                                {21, 20}, planet::affine::extents2d{88, 3}};
                check(std::get<2>(c.items).position())
                        == planet::affine::rectangle2d{
                                {111, 20}, planet::affine::extents2d{4, 3}};
            });


    auto const constraints = suite.test(
            "the expanding item is offered the left over width during reflow",
            [](auto check, auto &log) {
                auto c = planet::ui::expandable_row{
                        1,
                        std::tuple{
                                planet::debug::fixed_element{log, {4, 3}},
                                filling_element{3}},
                        2.0f};
                c.reflow({.screen = screen}, screen);

                check(c.constraints().extents())
                        == planet::affine::extents2d{100, 3};
            });


    auto const range = suite.test(
            "the expanding index must be an item", [](auto check, auto &log) {
                auto c = planet::ui::expandable_row{
                        1,
                        std::tuple{planet::debug::fixed_element{log, {4, 3}}}};
                check([&]() {
                    c.reflow({.screen = screen}, screen);
                }).template throws_type<felspar::stdexcept::logic_error>();
            });


}
