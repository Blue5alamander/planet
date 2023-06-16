#pragma once


#include <planet/serialise.hpp>

#include <atomic>


namespace planet::log {


    /// ## Log levels
    /**
     * Define a log level.
     * * `debug` -- intended for development or debugging logs.
     * * `info` -- informative logs.
     * * `warning` -- something is wrong.
     * * `error` -- there is an error, but it is recoverable.
     * * `critical` -- a non-recoverable error has occurred.
     */
    enum class level { debug, info, warning, error, critical };


    /// ## The currently active logging level

    /// ### Active logging level
    /// Any log messages below this level will always be discarded
    inline std::atomic<level> active{level::info};


    /// ## Log a message
    template<typename... Ms>
    void item(level, Ms &&...);


    /// ## Log messages at a given level
    template<typename... Ms>
    void debug(Ms &&...m) {
        item(level::debug, std::forward<Ms>(m)...);
    }
    template<typename... Ms>
    void info(Ms &&...m) {
        item(level::info, std::forward<Ms>(m)...);
    }
    template<typename... Ms>
    void warning(Ms &&...m) {
        item(level::warning, std::forward<Ms>(m)...);
    }
    template<typename... Ms>
    void error(Ms &&...m) {
        item(level::error, std::forward<Ms>(m)...);
    }
    template<typename... Ms>
    void critical(Ms &&...m) {
        item(level::critical, std::forward<Ms>(m)...);
    }


    namespace detail {
        inline thread_local serialise::save_buffer ab;
        void write_log(level, serialise::shared_bytes);
    }

    template<typename... Ms>
    void item(level const l, Ms &&...m) {
        (save(detail::ab, std::forward<Ms>(m)), ...);
        detail::write_log(l, detail::ab.complete());
    }


}
