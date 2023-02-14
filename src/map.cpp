#include <planet/map/square.hpp>
#include <planet/serialise.hpp>


/// ### `planet::map::coordinates`


planet::serialise::save_buffer &
        planet::map::save(serialise::save_buffer &ab, coordinates const c) {
    return ab.save_box("_p:m:coord", c.x, c.y);
}
void planet::map::load(std::span<std::byte const> &s, coordinates &c) {
    serialise::load_box(s, "_p:m:coord", c.x, c.y);
}


/// ### `planet::hexmap::coordinates`


planet::serialise::save_buffer &
        planet::hexmap::save(serialise::save_buffer &ab, coordinates const c) {
    return ab.save_box("_p:h:coord", c.pos);
}
