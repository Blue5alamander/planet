#include <planet/serialise/load_buffer.hpp>
#include <planet/serialise/save_buffer.hpp>
#include <planet/events.hpp>


namespace planet::events {


    /// ## `planet::events::back`
    inline void save(serialise::save_buffer &ab, back) {
        ab.save_box(back::box);
    }
    inline void load(serialise::load_buffer &lb, back &) {
        lb.load_box(back::box);
    }


    /// ## `planet::events::quit`
    inline void save(serialise::save_buffer &ab, quit) {
        ab.save_box(quit::box);
    }
    inline void load(serialise::load_buffer &lb, quit &) {
        lb.load_box(quit::box);
    }


}
