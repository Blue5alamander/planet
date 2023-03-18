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


auto planet::hexmap::best_direction(coordinates const from, coordinates const to)
        -> coordinates {
    auto const angle = (to - from).centre().theta();
    for (std::size_t index{}; auto const ref : angles) {
        if (std::abs(ref - angle) <= 1.0f / 12.0f) {
            return directions[index];
        } else {
            ++index;
        }
    }
    return south_east;
}


void planet::hexmap::save(serialise::save_buffer &ab, coordinates const c) {
    ab.save_box("_p:h:coord", c.column(), c.row());
}
void planet::hexmap::load(serialise::load_buffer &lb, coordinates &c) {
    coordinates::value_type col{}, row{};
    lb.load_box("_p:h:coord", col, row);
    c = {col, row};
}
