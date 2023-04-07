#pragma once


#include <planet/events/keys.hpp>
#include <planet/events/mouse.hpp>
#include <planet/events/quit.hpp>
#include <planet/events/scroll.hpp>

#include <felspar/coro/bus.hpp>


namespace planet::events {


    /// ## Events bus
    struct bus {
        /// ### Raw event busses
        felspar::coro::bus<events::key> key;
        felspar::coro::bus<events::mouse> mouse;
        felspar::coro::bus<events::quit> quit;
        felspar::coro::bus<events::scroll> scroll;
    };


}
