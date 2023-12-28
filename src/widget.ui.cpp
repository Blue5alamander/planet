#include <planet/ui/baseplate.hpp>
#include <planet/ui/widget.hpp>


void planet::ui::widget::deregister(ui::baseplate *const bp, widget *const w) {
    w->m_visible = false;
    if (bp) { bp->remove(w); }
}


void planet::ui::widget::add_to(ui::baseplate &bp, ui::panel &parent) {
    parent.add_child(panel);
    bp.add(this);
    m_visible = true;
    response.post(behaviour());
}
