#include <planet/debug/ui.hpp>
#include <planet/ostream.hpp>
#include <planet/ui/baseplate.hpp>
#include <planet/ui/layout.column.hpp>
#include <planet/ui/layout.row.hpp>
#include <planet/ui/screen.hpp>
#include <felspar/test.hpp>


namespace {


    using constrained_type = planet::ui::constrained2d<float>;

    constexpr planet::ui::reflowable::constrained_type screen{
            {40, 0, 40}, {30, 0, 30}};


    /**
     * A widget that records the hover durations it is notified with so the
     * hover routing can be observed. `last_was_clear` is true when the most
     * recent notification was a zero duration -- the "pointer left" signal.
     */
    struct hover_probe final : public planet::ui::widget {
        explicit hover_probe(std::string_view const n) : widget{n} {}

        std::chrono::steady_clock::duration last_duration{};
        bool last_was_clear{};

        void hover(std::chrono::steady_clock::duration const d) override {
            last_duration = d;
            last_was_clear = (d == std::chrono::steady_clock::duration{});
        }

        bool wants_focus() const noexcept override { return false; }

        constrained_type do_reflow(
                reflow_parameters const &, constrained_type const &c) override {
            return c;
        }
        planet::affine::rectangle2d do_move_sub_elements(
                reflow_parameters const &,
                planet::affine::rectangle2d const &r) override {
            return r;
        }
        void do_draw() override {}
        felspar::coro::task<void> behaviour() override { co_return; }
    };


    /**
     * A widget that contains another widget (a `debug::button`) which it moves
     * as part of its own `do_move_sub_elements`. Used to check that widget
     * containment raises the child onto a higher z layer than its parent.
     */
    struct containing_widget final : public planet::ui::widget {
        explicit containing_widget(std::ostream &log)
        : widget{"containing_widget"},
          child{planet::debug::fixed_element{log, {4, 3}}} {}

        planet::debug::button<planet::debug::fixed_element> child;

        constrained_type do_reflow(
                reflow_parameters const &p,
                constrained_type const &c) override {
            return child.reflow(p, c);
        }
        planet::affine::rectangle2d do_move_sub_elements(
                reflow_parameters const &p,
                planet::affine::rectangle2d const &r) override {
            return child.move_to(p, r);
        }
        void do_draw() override {}
        felspar::coro::task<void> behaviour() override { co_return; }
    };


    auto const suite = felspar::testsuite("baseplate");


    auto const ordering = suite.test("ordering", [](auto check, auto &log) {
        planet::ui::panel panel;
        planet::ui::baseplate bp;
        planet::debug::button<> btn{log};

        check(bp.widget_count()) == 0u;

        btn.add_to(bp, panel);

        check(bp.widget_count()) == 0u;

        check([&]() {
            btn.draw();
        }).throws(std::logic_error{"Reflowable position has not been set"});

        check([&]() {
            btn.move_to(
                    {.screen = screen},
                    {{15, 20}, planet::affine::extents2d{40, 30}});
        }).throws(std::logic_error{"Reflowable constraints have not been set"});
    });


    auto const events = suite.test(
            "events",
            [](auto check, auto &log) {
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                planet::debug::button<> btn{log};

                btn.add_to(bp, panel);
                btn.reflow({.screen = screen}, screen);
                btn.move_to(
                        {.screen = screen},
                        {{15, 20}, planet::affine::extents2d{4, 3}});
                btn.draw();
                check(btn.clicks) == 0u;

                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::down,
                         .location = {18, 21},
                         .clicks = 0});
                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::up,
                         .location = {18, 21},
                         .clicks = 1});
                check(btn.position())
                        == planet::affine::rectangle2d{
                                {15, 20}, planet::affine::extents2d{4, 3}};
                check(btn.clicks) == 1u;

                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::down,
                         .location = {8, 21}});
                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::up,
                         .location = {8, 21}});
                check(btn.clicks) == 1u;

                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::down,
                         .location = {18, 21}});
                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::up,
                         .location = {18, 21},
                         .clicks = 1});
                check(btn.clicks) == 2u;
            },
            [](auto check, auto &log) {
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                auto ui = planet::ui::column{std::tuple{
                        planet::ui::row{std::tuple{planet::debug::button{
                                planet::debug::fixed_element{log, {4, 3}}}}}}};

                auto &btn = std::get<0>(std::get<0>(ui.items).items);
                btn.add_to(bp, panel);
                ui.reflow({.screen = screen}, screen);
                ui.move_to(
                        {.screen = screen},
                        {{15, 20}, planet::affine::extents2d{4, 3}});
                ui.draw();

                log << "btn position: " << btn.position() << '\n';

                check(btn.wants_focus()) == true;

                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::down,
                         .location = {16, 21}});
                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::up,
                         .location = {16, 21},
                         .clicks = 1});
                check(btn.clicks) == 1u;
            });


    auto const hover_boundary =
            suite.test("hover boundary", [](auto check, auto &) {
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                hover_probe bottom{"bottom"}, top{"top"};
                bottom.add_to(bp, panel);
                top.add_to(bp, panel);

                /**
                 * Both probes occupy the same rectangle so both are under the
                 * pointer at once. The top probe is given a higher z layer so
                 * it is visited first when hover is routed.
                 */
                planet::affine::rectangle2d const box{
                        {15, 20}, planet::affine::extents2d{4, 3}};
                bottom.reflow({.screen = screen}, screen);
                bottom.move_to({.screen = screen}, box);
                top.reflow({.screen = screen}, screen);
                top.move_to({.screen = screen}, box);
                bottom.static_z_layer = 1.0f;
                top.static_z_layer = 2.0f;
                bottom.draw();
                top.draw();

                bp.events.mouse.push({.location = {16, 21}});
                bp.start_frame_reset();

                /// With no boundary set both widgets are hovered.
                check(bottom.last_was_clear) == false;
                check(top.last_was_clear) == false;

                /**
                 * Turn the top probe into a hover boundary and run another
                 * frame. The live list was cleared by the previous frame, so
                 * draw the probes again first.
                 */
                top.hover_boundary(true);
                bottom.draw();
                top.draw();
                bp.events.mouse.push({.location = {16, 21}});
                bp.start_frame_reset();

                /**
                 * The boundary is still hovered, but the widget below it is
                 * cleared as though the pointer had left.
                 */
                check(top.last_was_clear) == false;
                check(bottom.last_was_clear) == true;
            });


    auto const lifetime =
            suite.test("widget lifetimes", [](auto check, auto &log) {
                /**
                 * A widget bound through `add_to` may outlive the baseplate it
                 * was bound to — a screen kept in a static is the real case. As
                 * the screens do, bind to the baseplate's own `pixels` panel
                 * (the single-argument `add_to`), so the widget's panel parent
                 * lives and dies with the baseplate alongside its back-pointer.
                 */
                planet::debug::button<> btn{log};

                {
                    planet::ui::baseplate bp;
                    btn.add_to(bp);
                }

                /**
                 * The baseplate is gone. It must have nulled the widget's
                 * back-pointer on the way out, so binding afresh succeeds
                 * rather than reporting the widget is already attached — and,
                 * crucially, the widget's own later destruction will not
                 * deregister through the dangling baseplate.
                 */
                planet::ui::baseplate bp2;
                btn.add_to(bp2);
                btn.reflow({.screen = screen}, screen);
                btn.move_to(
                        {.screen = screen},
                        {{15, 20}, planet::affine::extents2d{4, 3}});
                btn.draw();
                check(bp2.widget_count()) == 1u;

                /// End the frame as the engine would, clearing the live list.
                bp2.start_frame_reset();
                check(bp2.widget_count()) == 0u;
            });


    auto const containment_raises_z =
            suite.test("containment raises z", [](auto check, auto &log) {
                /**
                 * A widget that moves a child widget as part of its own layout
                 * ends up below that child -- widget containment is the only
                 * thing that deepens z.
                 */
                containing_widget parent{log};
                parent.reflow({.screen = screen}, screen);
                parent.move_to(
                        {.screen = screen},
                        {{15, 20}, planet::affine::extents2d{4, 3}});

                check(parent.child.z_layer()) > parent.z_layer();
            });


    auto const layout_wrappers_inert =
            suite.test("layout wrappers inert", [](auto check, auto &log) {
                /**
                 * Rows and columns are layout nodes, not widgets, so they must
                 * not shift z. A button reached through `column{row{button}}`
                 * ties a sibling button moved directly at the top level.
                 */
                auto ui = planet::ui::column{std::tuple{
                        planet::ui::row{std::tuple{planet::debug::button{
                                planet::debug::fixed_element{log, {4, 3}}}}}}};
                auto &nested = std::get<0>(std::get<0>(ui.items).items);
                ui.reflow({.screen = screen}, screen);
                ui.move_to(
                        {.screen = screen},
                        {{15, 20}, planet::affine::extents2d{4, 3}});

                planet::debug::button top{
                        planet::debug::fixed_element{log, {4, 3}}};
                top.reflow({.screen = screen}, screen);
                top.move_to(
                        {.screen = screen},
                        {{15, 20}, planet::affine::extents2d{4, 3}});

                check(nested.z_layer()) == top.z_layer();
            });


    auto const default_depth_zero =
            suite.test("default reflow depth is zero", [](auto check, auto &) {
                /**
                 * The common `{.screen = ...}` call sites leave `depth` at its
                 * default so existing layout behaviour is unchanged.
                 */
                planet::ui::reflowable::reflow_parameters const params{
                        .screen = screen};
                check(params.depth) == 0.0f;
            });


    auto const z_survives_draw = suite.test(
            "containment z survives draw", [](auto check, auto &log) {
                /**
                 * `move_to` assigns the containment depth to
                 * `dynamic_z_layer`. `draw` must not overwrite it. The child is
                 * bound to a baseplate -- the path that used to copy
                 * `baseplate->z_layer` only runs when a widget has one -- then
                 * laid out one level deep by its parent and drawn. The depth
                 * value must come through intact.
                 */
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                containing_widget parent{log};
                parent.add_to(bp, panel);
                parent.child.add_to(bp, panel);

                parent.reflow({.screen = screen}, screen);
                parent.move_to(
                        {.screen = screen},
                        {{15, 20}, planet::affine::extents2d{4, 3}});

                /// The depth assigned by `move_to` before any drawing.
                check(parent.dynamic_z_layer) == 0.0f;
                check(parent.child.dynamic_z_layer) == 1.0f;

                parent.draw();
                parent.child.draw();

                /// The depth survives `draw` rather than being reset to zero.
                check(parent.child.dynamic_z_layer) == 1.0f;
                check(parent.child.z_layer()) > parent.z_layer();
            });


    auto const screens_layer_statically =
            suite.test("screens layer statically", [](auto check, auto &) {
                /**
                 * Screens cover everything, so nothing ever lays one out --
                 * they are added to the baseplate and drawn, never moved. That
                 * leaves the dynamic part of their z at zero, which is what
                 * makes the static value the whole of a screen's layer: a
                 * catch-all at -1 stays below every widget however shallow,
                 * and a modal at 100 stays above them however deep.
                 */
                planet::ui::baseplate bp;
                planet::ui::screen catch_all;
                planet::ui::screen modal{100.0f};
                catch_all.add_to(bp);
                modal.add_to(bp);
                catch_all.draw();
                modal.draw();

                check(catch_all.dynamic_z_layer) == 0.0f;
                check(catch_all.z_layer()) == -1.0f;
                check(modal.dynamic_z_layer) == 0.0f;
                check(modal.z_layer()) == 100.0f;
            });


    auto const contained_wins_click = suite.test(
            "contained widget wins the click",
            [](auto check, auto &log) {
                /**
                 * Parent drawn first, then its contained child. Both sit at
                 * static z zero and fully overlap, so only the containment
                 * depth separates them. The click must land on the contained
                 * child.
                 */
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                containing_widget parent{log};
                parent.add_to(bp, panel);
                parent.child.add_to(bp, panel);

                parent.reflow({.screen = screen}, screen);
                parent.move_to(
                        {.screen = screen},
                        {{15, 20}, planet::affine::extents2d{4, 3}});
                parent.draw();
                parent.child.draw();

                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::down,
                         .location = {16, 21}});
                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::up,
                         .location = {16, 21},
                         .clicks = 1});
                check(parent.child.clicks) == 1u;
            },
            [](auto check, auto &log) {
                /**
                 * The same set up but with the parent drawn after the child.
                 * With a depth tie the later-drawn parent would steal the soft
                 * focus, so this order is the one the containment depth has to
                 * rescue. The click still lands on the child.
                 */
                planet::ui::baseplate bp;
                planet::ui::panel panel;
                containing_widget parent{log};
                parent.add_to(bp, panel);
                parent.child.add_to(bp, panel);

                parent.reflow({.screen = screen}, screen);
                parent.move_to(
                        {.screen = screen},
                        {{15, 20}, planet::affine::extents2d{4, 3}});
                parent.child.draw();
                parent.draw();

                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::down,
                         .location = {16, 21}});
                bp.events.mouse.push(
                        {.button = planet::events::button::left,
                         .action = planet::events::action::up,
                         .location = {16, 21},
                         .clicks = 1});
                check(parent.child.clicks) == 1u;
            });


}
