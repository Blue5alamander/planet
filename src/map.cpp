#include <planet/map/square.hpp>
#include <planet/serialise.hpp>


/// ### `planet::map::coordinates`


void planet::map::save(serialise::save_buffer &ab, coordinates const c) {
    ab.save_box("_p:m:coord", c.x, c.y);
}
void planet::map::load(serialise::load_buffer &lb, coordinates &c) {
    lb.load_box("_p:m:coord", c.x, c.y);
}


/// ### `planet::hexmap::coordinates`


void planet::hexmap::save(serialise::save_buffer &ab, coordinates const c) {
    ab.save_box("_p:h:coord", c.pos);
}
void planet::hexmap::load(serialise::load_buffer &lb, coordinates &c) {
    lb.load_box("_p:h:coord", c.pos);
}
