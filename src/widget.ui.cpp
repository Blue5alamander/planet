#include <planet/ui/baseplate.hpp>
#include <planet/ui/widget.hpp>


void planet::ui::widget::deregister(ui::baseplate *const bp, widget *const w) {
    if (bp) { bp->remove(w); }
}


void planet::ui::widget::add_to(ui::baseplate &bp, ui::panel &parent) {
    if (baseplate) {
        throw felspar::stdexcept::logic_error{
                "This widget is already attached to a baseplate"};
    }
    baseplate = &bp;
    parent.add_child(panel);
    response.post(behaviour());
}


void planet::ui::widget::throw_invalid_add_to_target() {
    throw felspar::stdexcept::logic_error{
            "Cannot add this widget to one that isn't itself added to "
            "something"};
}
