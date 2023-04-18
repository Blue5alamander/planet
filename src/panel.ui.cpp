#include <planet/ui/panel.hpp>


/// ## `planet::panel`


planet::panel::panel() { feeder.post(*this, &panel::feed_children); }


planet::panel::panel(panel &&p, felspar::source_location const &loc) : panel{} {
    if (p.parent or not p.children.empty()) {
        throw felspar::stdexcept::logic_error{
                "A panel cannot be moved in memory (std::move) once it is in "
                "the hierarchy",
                loc};
    }
    viewport = p.viewport;
}


planet::panel::~panel() {
    if (parent) { parent->remove_child(*this); }
    reparent_children(parent);
}


felspar::coro::task<void> planet::panel::feed_children() {
    while (true) {
        auto click = co_await clicks.next();
        /**
         * We really only want to send the click to a single child of this
         * panel. Because we don't have Z heights or any other hierarchy at this
         * level we look for the smallest child that the mouse click is within
         * and send to that.
         */
        child *send_to = nullptr;
        for (auto &c : children) {
            if (c.area) {
                if (c.area->contains(click.location)) {
                    if (not send_to or send_to->area->contains(*c.area)) {
                        send_to = &c;
                    }
                }
            }
        }
        if (send_to) {
            auto transformed{click};
            transformed.location = send_to->sub->viewport.outof(click.location);
            send_to->sub->clicks.push(transformed);
        }
    }
}


void planet::panel::reparent_children(panel *const np) {
    for (auto &p : children) {
        p.sub->parent = np;
        if (np) {
            if (p.area) {
                np->add_child(*p.sub, *p.area);
            } else {
                np->add_child(*p.sub);
            }
        }
    }
}


planet::panel::child &planet::panel::add(panel *c) {
    c->parent = this;
    children.emplace_back(c);
    return children.back();
}
void planet::panel::add_child(panel &c) { add(&c); }
void planet::panel::add_child(panel &c, affine::rectangle2d const area) {
    add(&c).area = area;
    c.translate(area.top_left);
}


void planet::panel::remove_child(panel &c) {
    auto pos =
            std::find_if(children.begin(), children.end(), [&c](auto const &p) {
                return &c == p.sub;
            });
    if (pos != children.end()) {
        c.parent = nullptr;
        children.erase(pos);
    }
}


void planet::panel::move_to(affine::rectangle2d const area) {
    if (parent) {
        auto pos = std::find_if(
                parent->children.begin(), parent->children.end(),
                [this](auto const &p) { return p.sub == this; });
        if (pos != parent->children.end()) {
            if (pos->area) { translate(-pos->area->top_left); }
            translate(area.top_left);
            pos->area = area;
        }
    }
}


/// ## `planet::panel::child`


planet::panel::child::child(panel *const c) : area{}, sub{c} {}
planet::panel::child::child(panel *const c, affine::rectangle2d const a)
: area{a}, sub{c} {}
