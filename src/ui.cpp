#include <planet/ui/baseplate.hpp>
#include <planet/ui/draggable.hpp>
#include <planet/ui/widget.hpp>

#include <planet/widget/button.hpp>
#include <planet/widget/checkbox.hpp>
#include <planet/widget/volume.slider.hpp>


/// ## `planet::ui::drop_target`


void planet::ui::drop_target::start(constrained_type const &) {}


void planet::ui::drop_target::update(constrained_type const &) {}


/// ## `planet::ui::widget`


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
