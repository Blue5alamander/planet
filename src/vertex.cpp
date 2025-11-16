#include <planet/serialise.hpp>
#include <planet/vertex.hpp>


/// ## 'planet::vertex::coloured`


void planet::vertex::save(planet::serialise::save_buffer &sb, coloured const &v) {
    sb.save_box(v.box, v.p, v.col);
}
void planet::vertex::load(planet::serialise::box &box, coloured &v) {
    box.named(v.box, v.p, v.col);
}


/// ## `planet::vertex::coloured_textured`


void planet::vertex::save(
        serialise::save_buffer &sb, coloured_textured const &t) {
    sb.save_box(t.box, t.p, t.col, t.uv);
}
void planet::vertex::load(serialise::box &box, coloured_textured &t) {
    box.named(t.box, t.p, t.col, t.uv);
}


/// ## `planet::vertex::normal`


void planet::vertex::save(serialise::save_buffer &sb, normal const &n) {
    sb.save_box(n.box, n.p, n.n);
}
void planet::vertex::load(serialise::box &box, normal &n) {
    box.named(n.box, n.p, n.n);
}


/// ## `planet::vertex::normal_textured`


static_assert(sizeof(planet::vertex::normal_textured::p) == 16);
static_assert(sizeof(planet::vertex::normal_textured::n) == 16);
static_assert(sizeof(planet::vertex::normal_textured::uv) == 8);
static_assert(sizeof(planet::vertex::normal_textured) == 40);


void planet::vertex::save(serialise::save_buffer &sb, normal_textured const &n) {
    sb.save_box(n.box, n.p, n.n, n.uv);
}
void planet::vertex::load(serialise::box &box, normal_textured &n) {
    box.named(n.box, n.p, n.n, n.uv);
}


/// ## `planet::vertex::uvpos`


void planet::vertex::save(serialise::save_buffer &sb, uvpos const &p) {
    sb.save_box(p.box, p.u, p.v);
}
void planet::vertex::load(serialise::box &box, uvpos &p) {
    box.named(p.box, p.u, p.v);
}
