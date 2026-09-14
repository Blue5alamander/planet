#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/layout.expandable_column.hpp>
#include <felspar/test.hpp>


namespace {


    /// ## An element that takes up all of the height it is offered
    struct filling_element final : public planet::ui::reflowable {
        float width;


        filling_element(float const w)
        : reflowable{"filling_element"}, width{w} {}


      private:
        constrained_type do_reflow(
                reflow_parameters const &, constrained_type const &c) override {
            return {width, c.height.value()};
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


    auto const suite = felspar::testsuite("expandable_column.layout");


    auto const declined =
            suite.test("the offer is declined", [](auto check, auto &log) {
                /// The column is then laid out exactly as a `column` would be
                auto c = planet::ui::expandable_column{
                        0,
                        std::tuple{
                                planet::debug::fixed_element{log, {3, 4}},
                                planet::debug::fixed_element{log, {3, 4}}},
                        2.0f};
                c.reflow({.screen = screen}, screen);
                c.move_to({.screen = screen}, full);

                check(c.position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{3, 10}};
                check(std::get<0>(c.items).position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{3, 4}};
                check(std::get<1>(c.items).position())
                        == planet::affine::rectangle2d{
                                {15, 26}, planet::affine::extents2d{3, 4}};
            });


    auto const taken =
            suite.test("the offer is taken", [](auto check, auto &log) {
                auto c = planet::ui::expandable_column{
                        0,
                        std::tuple{
                                filling_element{3},
                                planet::debug::fixed_element{log, {3, 4}}},
                        2.0f};
                c.reflow({.screen = screen}, screen);
                c.move_to({.screen = screen}, full);

                check(c.position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{3, 100}};
                check(std::get<0>(c.items).position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{3, 94}};
                check(std::get<1>(c.items).position())
                        == planet::affine::rectangle2d{
                                {15, 116}, planet::affine::extents2d{3, 4}};
            });


    auto const middle =
            suite.test("expanding in the middle", [](auto check, auto &log) {
                auto c = planet::ui::expandable_column{
                        1,
                        std::tuple{
                                planet::debug::fixed_element{log, {3, 4}},
                                filling_element{3},
                                planet::debug::fixed_element{log, {3, 4}}},
                        2.0f};
                c.reflow({.screen = screen}, screen);
                c.move_to({.screen = screen}, full);

                check(c.position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{3, 100}};
                check(std::get<0>(c.items).position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{3, 4}};
                check(std::get<1>(c.items).position())
                        == planet::affine::rectangle2d{
                                {15, 26}, planet::affine::extents2d{3, 88}};
                check(std::get<2>(c.items).position())
                        == planet::affine::rectangle2d{
                                {15, 116}, planet::affine::extents2d{3, 4}};
            });


    auto const constraints = suite.test(
            "the expanding item is offered the left over height during reflow",
            [](auto check, auto &log) {
                auto c = planet::ui::expandable_column{
                        1,
                        std::tuple{
                                planet::debug::fixed_element{log, {3, 4}},
                                filling_element{3}},
                        2.0f};
                c.reflow({.screen = screen}, screen);

                check(c.constraints().extents())
                        == planet::affine::extents2d{3, 100};
            });


    auto const range = suite.test(
            "the expanding index must be an item", [](auto check, auto &log) {
                auto c = planet::ui::expandable_column{
                        1,
                        std::tuple{planet::debug::fixed_element{log, {3, 4}}}};
                check([&]() {
                    c.reflow({.screen = screen}, screen);
                }).template throws_type<felspar::stdexcept::logic_error>();
            });


}
