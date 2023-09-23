#pragma once


#include <planet/events/keys.hpp>
#include <planet/events/mouse.hpp>
#include <planet/events/quit.hpp>
#include <planet/events/scroll.hpp>
#include <planet/queue/pmc.hpp>


namespace planet::events {


    /// ## Events bus
    struct bus {
        /// ### Raw event busses
        queue::pmc<events::key> key;
        queue::pmc<events::mouse> mouse;
        queue::pmc<events::quit> quit;
        queue::pmc<events::scroll> scroll;
    };


}
