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
    enum class level : std::uint8_t { debug, info, warning, error, critical };
    inline auto operator<=>(level const l, level const r) {
        return static_cast<std::uint8_t>(l) <=> static_cast<std::uint8_t>(r);
    }


    /// ## The currently active logging level

    /// ### Active logging level
    /**
     * Any log messages below this level will always be discarded. The value can
     * be updated at any time and the change will have immediate effect.
     */
    inline std::atomic<level> active{level::debug};


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
    [[noreturn]] void critical(Ms &&...m) {
        item(level::critical, std::forward<Ms>(m)...);
        /**
         * Wait for a bit here.
         *
         * The terminate that is actually meaningful is the one in the
         * [implementation file](../../src/log.cpp) which will cause the program
         * to terminate after dealing with this log message. The one here is
         * just to ensure that this function doesn't actually return.
         */
        ::sleep(2);
        std::terminate();
    }


    namespace detail {
        inline thread_local serialise::save_buffer ab;
        void write_log(level, serialise::shared_bytes);
    }

    template<typename... Ms>
    void item(level const l, Ms &&...m) {
        if (l >= active.load()) {
            (save(detail::ab, std::forward<Ms>(m)), ...);
            detail::write_log(l, detail::ab.complete());
        }
    }


    /// ## Log messagesstd
    struct message {
        planet::log::level level;
        planet::serialise::shared_bytes payload;
        std::chrono::steady_clock::time_point logged =
                std::chrono::steady_clock::now();
    };


    /// ## Custom log message formatter
    namespace detail {
        struct formatter {
            formatter(std::string_view);
            ~formatter();
            std::string_view const box_name;
            virtual void print(std::ostream &, serialise::box &) const = 0;
        };
    }
    template<typename Lambda>
    auto format(std::string_view const box_name, Lambda lambda) {
        struct printer : public detail::formatter {
            printer(std::string_view const n, Lambda l)
            : formatter{n}, lambda{std::move(l)} {}
            Lambda lambda;
            void print(std::ostream &os, serialise::box &box) const override {
                lambda(os, box);
            }
        };
        return printer{box_name, std::move(lambda)};
    }


}
