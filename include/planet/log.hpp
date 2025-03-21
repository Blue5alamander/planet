#pragma once


#include <planet/serialise.hpp>

#include <atomic>


namespace planet::log {


    /// # log.hpp


    /// ## Stop the logging thread
    void stop_thread();
    /**
     * The logging thread is started automatically when the first log message is
     * sent and will be stopped when the program terminates. This function can
     * be used to stop the thread during normal shut-down and will cause a final
     * print of performance data. Normally you'd want to call it after all other
     * code has run.
     */


    /// ## Log levels
    enum class level : std::uint8_t { debug, info, warning, error, critical };
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


    /// ### Active logging

    /// #### Log level
    inline std::atomic<level> active{level::debug};
    /**
     * Any log messages below this level will always be discarded.
     *
     * The value can be updated at any time and the change will have immediate
     * effect. Any log messages already in flight will still arrive.
     */

    /// #### Performance data print out
    inline std::atomic<bool> display_performance_messages{true};


    /// #### Log output file
    inline std::atomic<std::ostream *> log_output = nullptr;
    /**
     * There are separate files for logging and performance data. There would
     * normally be different upload conditions for performance and log file data.
     *
     * When set any log and performance data is sent to the pointed to file.
     * This may not be changed or cleared once log messages are being sent, so
     * set it once early during start up and then never change it.
     *
     * TODO Change to use an atomic shared pointer so that it can be changed at
     * any time (i.e. log files can be rotated).
     */


    namespace detail {
        extern thread_local serialise::save_buffer ab;
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
    struct item final {
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
    struct debug final {
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
    struct info final {
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
    struct warning final {
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
    struct error final {
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

    namespace detail {
        [[noreturn]] void critical_log_encountered();
    }
    template<typename... Ms>
    struct critical final {
        [[noreturn]] critical(
                Ms const &...m,
                felspar::source_location const &loc =
                        felspar::source_location::current()) {
            (detail::log(detail::ab, m), ...);
            detail::write_log(level::critical, detail::ab.complete(), loc);
            detail::critical_log_encountered();
        }
    };
    template<typename... Ms>
    critical(Ms...) -> critical<Ms...>;


    /// ## Log message storage
    struct message final {
        static constexpr std::string_view box{"_p:log:m"};


        log::level level;
        serialise::shared_bytes payload;
        felspar::source_location location;
        std::chrono::steady_clock::time_point logged =
                std::chrono::steady_clock::now();
    };
    void save(serialise::save_buffer &, message);


    /// ## Pretty printing
    void pretty_print(
            serialise::load_buffer &,
            std::size_t depth = 1,
            std::string_view prefix = " ");
    void pretty_print(
            serialise::box &,
            std::size_t depth = 1,
            std::string_view prefix = " ");
    /**
     * Walks along the data held in the load buffer pretty printing anything
     * that it finds, or pretty print a single box.
     *
     * The `depth` describes how deeply nested the current print is, with the
     * `prefix` being pre-pended to each line output once for each tick of
     * depth.
     */


    namespace detail {
        struct formatter {
            formatter(std::string_view);
            ~formatter();
            std::string_view const box_name;
            virtual void print(std::ostream &, serialise::box &) const = 0;
        };
    }


    /// ### Custom log message formatter
    template<typename Lambda>
    auto format(std::string_view const box_name, Lambda lambda) {
        struct printer final : public detail::formatter {
            printer(std::string_view const n, Lambda l)
            : formatter{n}, lambda{std::move(l)} {}
            Lambda lambda;
            void print(std::ostream &os, serialise::box &box) const override {
                lambda(os, box);
            }
        };
        return printer{box_name, std::move(lambda)};
    }
    /**
     * Create a `const` global of this type, passing it the box name and the
     * lambda that is to be used to print the box. The lambda will be called
     * with a `std::ostream &` and a `box `
     */


    /// ## Log message counter values

    /// ### Counts of the current number of log messages seen
    struct counters {
        std::int64_t debug, info, warning, error;

        static counters current() noexcept;
    };


    /// ### Performance counters saved to the log file
    struct logged_performance_counters {
        static constexpr std::string_view box{"_p:log:c"};


        serialise::shared_bytes counters;
        std::chrono::steady_clock::time_point logged =
                std::chrono::steady_clock::now();


        /// #### Print performance counters
        /**
         * Prints the performance counters found in the bytes. The data will be
         * transferred to the logging thread for printing so it doesn't
         * interfere with the printing of other log messages.
         */
        static void print(serialise::shared_bytes);
    };
    void save(serialise::save_buffer &, logged_performance_counters);


    /// ## Log file header

    struct file_header {
        static constexpr std::string_view box{"_p:log:h"};

        /// ### Time stamp the game started
        std::chrono::steady_clock::time_point base_time = {};
        /// ### Common file prefix for source location
        std::string file_prefix = {};
    };
    void load_fields(serialise::box &, file_header &);


    /// ### Write log file header

    /// #### To `save_buffer`
    void write_file_headers(serialise::save_buffer &);
    /**
     * Write the `file_header` structure, filled in with the relevant logging
     * information, to the save buffer.
     */

    /// #### To the current log files
    void write_file_headers();
    /**
     * Calling this will write a header to the log file containing information
     * helpful for interpreting the log file. Typically you'd want to call this
     * just after opening a log file and before sending any more log messages,
     * that way the header will appear first in the file.'
     */


}
