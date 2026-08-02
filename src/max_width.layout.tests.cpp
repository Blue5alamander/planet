#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/max_width.hpp>
#include <felspar/test.hpp>


namespace {


    constexpr planet::ui::reflowable::constrained_type screen{
            {400, 0, 400}, {300, 0, 300}};


    /// ## Content that takes everything it is offered
    /**
     * The constraint it was handed is kept so the tests can look at what the
     * cap did to it on the way down.
     */
    struct fill_element final : public planet::ui::reflowable {
        fill_element() : reflowable{"fill_element"} {}


        constrained_type offered = {};


      private:
        constrained_type do_reflow(
                reflow_parameters const &, constrained_type const &c) override {
            offered = c;
            return c;
        }
        planet::affine::rectangle2d move_sub_elements(
                reflow_parameters const &,
                planet::affine::rectangle2d const &r) override {
            return r;
        }
    };


    auto const suite = felspar::testsuite("max_width");


    auto const narrows = suite.test("narrows", [](auto check) {
        auto m = planet::ui::max_width{fill_element{}, 100.0f};
        m.reflow({.screen = screen}, screen);

        check(m.content.offered.width.max()) == 100.0f;
        check(m.content.offered.width.value()) == 100.0f;
        check(m.constraints().extents()) == planet::affine::extents2d{100, 300};
    });


    auto const passes_through = suite.test(
            "passes_through",
            [](auto check) {
                /// A cap wider than the space on offer changes nothing
                auto m = planet::ui::max_width{fill_element{}, 1000.0f};
                m.reflow({.screen = screen}, screen);

                check(m.content.offered.width.max()) == 400.0f;
                check(m.content.offered.width.value()) == 400.0f;
                check(m.content.offered.height.max()) == 300.0f;
            },
            [](auto check) {
                /// The heights are left alone when the width is capped
                auto m = planet::ui::max_width{fill_element{}, 100.0f};
                m.reflow({.screen = screen}, screen);

                check(m.content.offered.height.min()) == 0.0f;
                check(m.content.offered.height.value()) == 300.0f;
                check(m.content.offered.height.max()) == 300.0f;
            },
            [](auto check) {
                /// The minimum width is the content's to ask for, not the cap's
                auto m = planet::ui::max_width{fill_element{}, 100.0f};
                m.reflow({.screen = screen}, {{400, 20, 400}, {300, 0, 300}});

                check(m.content.offered.width.min()) == 20.0f;
            });


    auto const smaller_content =
            suite.test("smaller_content", [](auto check, auto &log) {
                /// Content narrower than the cap keeps its own size
                constexpr planet::affine::extents2d size{40, 30};

                auto m = planet::ui::max_width{
                        planet::debug::fixed_element{log, size}, 100.0f};
                m.reflow({.screen = screen}, screen);
                m.move_to({.screen = screen}, {{}, size});

                check(m.constraints().extents()) == size;
                check(m.position())
                        == planet::affine::rectangle2d{{0, 0}, size};
            });


    auto const minimum_wins = suite.test("minimum_wins", [](auto check) {
        /**
         * A container that insists on more width than the cap gets it: the cap
         * is held at the minimum rather than being taken below it, which would
         * leave a constraint that no width can satisfy.
         */
        auto m = planet::ui::max_width{fill_element{}, 100.0f};
        m.reflow({.screen = screen}, {{400, 200, 400}, {300, 0, 300}});

        check(m.content.offered.width.min()) == 200.0f;
        check(m.content.offered.width.max()) == 200.0f;
        check(m.content.offered.width.value()) == 200.0f;
    });


    auto const moves = suite.test("moves", [](auto check) {
        /// A stretching container cannot take the content past the cap
        constexpr planet::affine::point2d position{50, 75};

        auto m = planet::ui::max_width{fill_element{}, 100.0f};
        m.reflow({.screen = screen}, screen);
        m.move_to(
                {.screen = screen},
                {position, planet::affine::extents2d{400, 300}});

        check(m.position())
                == planet::affine::rectangle2d{
                        position, planet::affine::extents2d{100, 300}};
        check(m.content.position())
                == planet::affine::rectangle2d{
                        position, planet::affine::extents2d{100, 300}};
    });


}
