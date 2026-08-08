#include <planet/ui/baseplate.hpp>
#include <planet/ui/draggable.hpp>
#include <planet/ui/widget.hpp>

#include <felspar/coro/starter.hpp>


/// ## `planet::ui::drop_target`


void planet::ui::drop_target::start(constrained_type const &) {}


void planet::ui::drop_target::update(constrained_type const &) {}


/// ## `planet::ui::widget`


void planet::ui::widget::deregister(ui::baseplate *const bp, widget *const w) {
    if (bp) { bp->remove(w); }
}


namespace {
    /**
     * Draining matters as much as subscribing: an un-read consumer buffers
     * every event it is sent for as long as it is subscribed.
     */
    template<typename Queue>
    felspar::coro::task<void> drop_events(Queue &q) {
        auto events = q.values();
        while (true) { co_await events.next(); }
    }
}
felspar::coro::task<void>
        planet::ui::widget::swallow(events::kinds const kinds) {
    /**
     * A coroutine can only wait on one queue, so each kind gets its own. They
     * are held in a starter in this frame, which puts them in the behaviour
     * the widget posts -- so re-posting after a move brings them all back.
     */
    felspar::coro::starter<> drains;
    if (kinds.mouse) { drains.post(drop_events(events.mouse)); }
    if (kinds.key) { drains.post(drop_events(events.key)); }
    if (kinds.scroll) { drains.post(drop_events(events.scroll)); }
    if (kinds.text) { drains.post(drop_events(events.text)); }
    /// No drain ever finishes, so with any kind named this waits for good
    if (not drains.empty()) { co_await drains.wait_for_all(); }
}


void planet::ui::widget::add_to(ui::baseplate &bp, ui::panel &parent) {
    if (baseplate) {
        throw felspar::stdexcept::logic_error{
                "This widget is already attached to a baseplate"};
    }
    baseplate = &bp;
    bp.attach(this);
    parent.add_child(panel);
    response.post(behaviour());
}


void planet::ui::widget::throw_invalid_add_to_target() {
    throw felspar::stdexcept::logic_error{
            "Cannot add this widget to one that isn't itself added to "
            "something"};
}
