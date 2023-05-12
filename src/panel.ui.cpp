#include <planet/ui/panel.hpp>


/// ## `panel::ui::panel`


planet::ui::panel::panel(panel &&p, felspar::source_location const &loc)
: panel{} {
    if (p.m_parent or not p.children.empty()) {
        throw felspar::stdexcept::logic_error{
                "A panel cannot be moved in memory (std::move) once it is in "
                "the hierarchy",
                loc};
    }
    viewport = p.viewport;
}


planet::ui::panel::~panel() {
    if (m_parent) { m_parent->remove_child(*this); }
    reparent_children(m_parent);
}


void planet::ui::panel::reparent_children(panel *const np) {
    for (auto &p : children) {
        p.sub->m_parent = np;
        if (np) {
            if (p.area) {
                np->add_child(*p.sub, *p.area);
            } else {
                np->add_child(*p.sub);
            }
        }
    }
}


planet::ui::panel::child &planet::ui::panel::add(panel *c) {
    c->m_parent = this;
    children.emplace_back(c);
    return children.back();
}
void planet::ui::panel::add_child(panel &c) { add(&c); }
void planet::ui::panel::add_child(panel &c, affine::rectangle2d const area) {
    add(&c).area = area;
    c.translate(area.top_left);
}


void planet::ui::panel::remove_child(panel &c) {
    auto pos =
            std::find_if(children.begin(), children.end(), [&c](auto const &p) {
                return &c == p.sub;
            });
    if (pos != children.end()) {
        c.m_parent = nullptr;
        children.erase(pos);
    }
}


void planet::ui::panel::move_to(affine::rectangle2d const &area) {
    if (m_parent) {
        auto pos = std::find_if(
                m_parent->children.begin(), m_parent->children.end(),
                [this](auto const &p) { return p.sub == this; });
        if (pos != m_parent->children.end()) {
            if (pos->area) { translate(-pos->area->top_left); }
            translate(area.top_left);
            pos->area = area;
        }
    }
}


/// ## `planet::ui::panel::child`


planet::ui::panel::child::child(panel *const c) : area{}, sub{c} {}
planet::ui::panel::child::child(panel *const c, affine::rectangle2d const a)
: area{a}, sub{c} {}
