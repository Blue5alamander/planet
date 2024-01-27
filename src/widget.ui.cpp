#include <planet/ui/baseplate.hpp>
#include <planet/ui/widget.hpp>


void planet::ui::widget::deregister(ui::baseplate *const bp, widget *const w) {
    if (bp) { bp->remove(w); }
}


void planet::ui::widget::add_to(ui::baseplate &bp, ui::panel &parent) {
    baseplate = &bp;
    parent.add_child(panel);
    response.post(behaviour());
}
