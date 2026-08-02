#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/widget/button.hpp>

#include <felspar/test.hpp>


namespace {


    constexpr planet::ui::reflowable::constrained_type screen{
            {40, 0, 40}, {30, 0, 30}};


    /// Lay a button out over a known rectangle and register it for input
    template<typename Button>
    void place(planet::ui::baseplate &bp, Button &btn) {
        btn.add_to(bp);
        btn.reflow({.screen = screen}, screen);
        btn.move_to(
                {.screen = screen},
                {{15, 20}, planet::affine::extents2d{4, 3}});
        btn.draw();
    }

    /// A press of the left button over the rectangle `place` gives a button
    void
            press(planet::ui::baseplate &bp,
                  planet::events::modifiers const modifiers = {}) {
        bp.events.mouse.push(
                {.button = planet::events::button::left,
                 .action = planet::events::action::down,
                 .location = {18, 21},
                 .modifiers = modifiers});
        bp.events.mouse.push(
                {.button = planet::events::button::left,
                 .action = planet::events::action::up,
                 .location = {18, 21},
                 .clicks = 1,
                 .modifiers = modifiers});
    }


    auto const suite = felspar::testsuite("button.ui");


    auto const callback = suite.test("callback", [](auto check, auto &log) {
        /// A callback taking nothing is told only that the button was pressed
        planet::ui::baseplate bp;
        std::size_t presses{};
        auto btn = planet::widget::button{
                "clicked", planet::debug::fixed_element{log, {4, 3}},
                [&presses]() { ++presses; }};
        place(bp, btn);

        press(bp);
        check(presses) == 1u;
        press(bp, {.shift = true});
        check(presses) == 2u;
    });


    auto const future = suite.test("future", [](auto check, auto &log) {
        /**
         * A press of a button delivering into a future carries no value, so
         * the future is a `future<void>` and the first press is the whole of
         * what it delivers: the value is set and the behaviour ends, leaving
         * later presses with nothing to do.
         */
        planet::ui::baseplate bp;
        felspar::coro::future<void> pressed;
        auto btn = planet::widget::button{
                "clicked", planet::debug::fixed_element{log, {4, 3}}, pressed};
        place(bp, btn);
        check(pressed.has_value()) == false;

        press(bp);
        check(pressed.has_value()) == true;
        /// A second press sets nothing, which a set future would throw over
        press(bp);
        check(pressed.has_value()) == true;
    });


    auto const clicks = suite.test("click-callback", [](auto check, auto &log) {
        /**
         * A callback taking a click is handed the whole event, so the
         * modifiers held at the time are there to be read and a shift-click
         * can be told from the plain kind.
         */
        planet::ui::baseplate bp;
        std::vector<planet::events::click> presses;
        auto btn = planet::widget::button{
                "clicked", planet::debug::fixed_element{log, {4, 3}},
                [&presses](planet::events::click const c) {
                    presses.push_back(c);
                }};
        place(bp, btn);

        press(bp);
        check(presses.size()) == 1u;
        check(presses.back().modifiers == planet::events::modifiers{}) == true;
        check(presses.back().count) == 1u;
        check(presses.back().location) == planet::affine::point2d{18, 21};

        press(bp, {.shift = true});
        check(presses.size()) == 2u;
        check(presses.back().modifiers
              == planet::events::modifiers{.shift = true})
                == true;
    });


}
