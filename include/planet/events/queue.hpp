#pragma once


#include <planet/events/focus.hpp>
#include <planet/events/keys.hpp>
#include <planet/events/mouse.hpp>
#include <planet/events/position.hpp>
#include <planet/events/quit.hpp>
#include <planet/events/resize.hpp>
#include <planet/events/scroll.hpp>
#include <planet/queue/pmc.hpp>


namespace planet::events {


    /// ## Events bus
    struct queue {
        /// ### Raw event queues
        planet::queue::pmc<events::focus> focus;
        planet::queue::pmc<events::key> key;
        planet::queue::pmc<events::mouse> mouse;
        planet::queue::pmc<events::window_pointer> pointer;
        planet::queue::pmc<events::position> position;
        planet::queue::pmc<events::quit> quit;
        planet::queue::pmc<events::resize> resize;
        planet::queue::pmc<events::scroll> scroll;
    };


}
