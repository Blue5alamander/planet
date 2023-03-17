#pragma once


namespace planet::behaviour {


    /// ## State for a behaviour
    /**
     * We don't need the `running` state because that's implicit in the
     * coroutine implementation anyway.
     */
    enum class state : bool { failure, success };


}
