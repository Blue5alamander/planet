#include <planet/events/mouse.hpp>


bool planet::events::is_click(
        mouse_settings const &conf, mouse const &e1, mouse const &e2) {
    return e1.which == e2.which and e1.pressed == mouse::state::down
            and e2.pressed == mouse::state::up
            and e2.timestamp - e1.timestamp <= conf.click_time;
}
