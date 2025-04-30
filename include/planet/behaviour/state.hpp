#pragma once


#include <expected>


namespace planet::behaviour {


    /// ## State for a behaviour
    /**
     * We don't need the `running` state because that's implicit in the
     * coroutine implementation anyway.
     */
    struct failure {};

    template<typename R>
    using state = std::expected<R, failure>;


}
