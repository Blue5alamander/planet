#pragma once


#include <typeinfo>


namespace planet::behaviour {


    /// ## Context value look-up key
    struct type {
        std::type_info const *id;
    };


}
