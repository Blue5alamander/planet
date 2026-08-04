#include <planet/serialise.hpp>
#include <planet/text/configuration.hpp>


void planet::text::save(
        planet::serialise::save_buffer &sb, configuration const &c) {
    sb.save_box(
            c.box, c.commit, c.cancel, c.erase_backwards, c.erase_forwards,
            c.caret_left, c.caret_right, c.caret_to_start, c.caret_to_end);
}
void planet::text::load(planet::serialise::box &box, configuration &c) {
    box.named(
            c.box, c.commit, c.cancel, c.erase_backwards, c.erase_forwards,
            c.caret_left, c.caret_right, c.caret_to_start, c.caret_to_end);
}
