#pragma once


#include <expected>
#include <string_view>


namespace planet::behaviour {


    /// ## State for a behaviour
    /**
     * We don't need the `running` state because that's implicit in the
     * coroutine implementation anyway.
     */
    struct failure {
        std::string_view reason;


        friend bool operator==(failure const &, failure const &) = default;
    };


    template<typename R>
    using state = std::expected<R, failure>;


}
