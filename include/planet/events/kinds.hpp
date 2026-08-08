#pragma once


namespace planet::events {


    /// ## A set of the routable event kinds
    /**
     * Names some subset of the four kinds that are routed to widgets -- the
     * members of `events::queue` that a `ui::baseplate` delivers rather than
     * the window events the application drains itself.
     *
     * Two questions are asked with it, and they turn out to be the same
     * question:
     *
     * - which kinds a hard focus captures, so they go to the widget holding it
     *   whatever the pointer is over rather than being routed by position (see
     *   `ui::widget::hard_focus_on`);
     * - which kinds a widget swallows, so their walk down the delivery stack
     *   ends there (see `ui::widget::swallow`).
     *
     * Empty unless kinds are named, so `kinds::all()` is how a widget asks for
     * the lot.
     */
    struct kinds final {
        bool mouse = false, key = false, scroll = false, text = false;

        /// ### The kinds that happen at a place
        static constexpr kinds pointer() noexcept {
            return {.mouse = true, .scroll = true};
        }
        /// ### The kinds that happen at the keyboard
        static constexpr kinds keyboard() noexcept {
            return {.key = true, .text = true};
        }
        /// ### Every kind
        static constexpr kinds all() noexcept {
            return {.mouse = true, .key = true, .scroll = true, .text = true};
        }
    };


}
