#pragma once


#include <planet/serialise.hpp>

#include <atomic>


namespace planet::log {


    /// ## Log levels
    /**
     * Defined log levels:
     *
     * * `debug` -- intended for development or debugging logs.
     * * `info` -- informative logs.
     * * `warning` -- something is wrong.
     * * `error` -- there is an error, but it is recoverable.
     * * `critical` -- a non-recoverable error has occurred.
     *
     * Critical errors will result in the termination of the process sending the
     * log message.
     */
    enum class level : std::uint8_t { debug, info, warning, error, critical };
    inline auto operator<=>(level const l, level const r) {
        return static_cast<std::uint8_t>(l) <=> static_cast<std::uint8_t>(r);
    }

    inline void save(serialise::save_buffer &sb, level const l) {
        sb.append(static_cast<std::uint8_t>(l));
    }
    inline void load(serialise::load_buffer &lb, level &l) {
        l = level{lb.extract<std::uint8_t>()};
    }


    /// ## The currently active logging level


    /// ### Active logging level
    /**
     * Any log messages below this level will always be discarded.
     *
     * The value can be updated at any time and the change will have immediate
     * effect. Any log messages already in flight will still arrive.
     */
    inline std::atomic<level> active{level::debug};


    namespace detail {
        inline thread_local serialise::save_buffer ab;
        void write_log(
                level,
                serialise::shared_bytes,
                felspar::source_location const &);

        template<typename T>
        concept has_own_log_implementation =
                requires(serialise::save_buffer sb, T const &t) {
                    { t.log(sb) };
                };
        template<typename T>
        void log(serialise::save_buffer &sb, T const &v) {
            save(sb, v);
        }
        template<has_own_log_implementation T>
        void log(serialise::save_buffer &sb, T const &v) {
            v.log(sb);
        }
    }


    /// ## Log a message
    template<typename... Ms>
    struct item {
        item(level const l,
             Ms &&...m,
             felspar::source_location const &loc =
                     felspar::source_location::current()) {
            if (l >= active.load()) {
                (detail::log(detail::ab, std::forward<Ms>(m)), ...);
                detail::write_log(l, detail::ab.complete(), loc);
            }
        }
    };
    template<typename... Ms>
    item(level, Ms...) -> item<Ms...>;


    /// ## Log messages at a given level
    template<typename... Ms>
    struct debug {
        debug(Ms const &...m,
              felspar::source_location const &loc =
                      felspar::source_location::current()) {
            if (level::debug >= active.load()) {
                (detail::log(detail::ab, m), ...);
                detail::write_log(level::debug, detail::ab.complete(), loc);
            }
        }
    };
    template<typename... Ms>
    debug(Ms...) -> debug<Ms...>;

    template<typename... Ms>
    struct info {
        info(Ms const &...m,
             felspar::source_location const &loc =
                     felspar::source_location::current()) {
            if (level::info >= active.load()) {
                (detail::log(detail::ab, m), ...);
                detail::write_log(level::info, detail::ab.complete(), loc);
            }
        }
    };
    template<typename... Ms>
    info(Ms...) -> info<Ms...>;

    template<typename... Ms>
    struct warning {
        warning(Ms const &...m,
                felspar::source_location const &loc =
                        felspar::source_location::current()) {
            if (level::warning >= active.load()) {
                (detail::log(detail::ab, m), ...);
                detail::write_log(level::warning, detail::ab.complete(), loc);
            }
        }
    };
    template<typename... Ms>
    warning(Ms...) -> warning<Ms...>;

    template<typename... Ms>
    struct error {
        error(Ms const &...m,
              felspar::source_location const &loc =
                      felspar::source_location::current()) {
            if (level::error >= active.load()) {
                (detail::log(detail::ab, m), ...);
                detail::write_log(level::error, detail::ab.complete(), loc);
            }
        }
    };
    template<typename... Ms>
    error(Ms...) -> error<Ms...>;

    template<typename... Ms>
    struct critical {
        [[noreturn]] critical(
                Ms const &...m,
                felspar::source_location const &loc =
                        felspar::source_location::current()) {
            (detail::log(detail::ab, m), ...);
            detail::write_log(level::critical, detail::ab.complete(), loc);
            /**
             * Wait for a bit here.
             *
             * The terminate that is actually meaningful is the one in the
             * [implementation file](../../src/log.cpp) which will cause the
             * program to terminate after dealing with this log message. The one
             * here is just to ensure that this function doesn't actually return.
             */
            ::sleep(2);
            std::exit(121);
        }
    };
    template<typename... Ms>
    critical(Ms...) -> critical<Ms...>;


    /// ## Log message storage
    struct message {
        log::level level;
        serialise::shared_bytes payload;
        felspar::source_location location;
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
