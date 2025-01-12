#pragma once


#include <planet/telemetry/performance.hpp>

#include <felspar/coro/eager.hpp>
#include <felspar/coro/task.hpp>
#include <felspar/io/warden.hpp>

#include <chrono>


namespace planet::telemetry {


    class time;
    namespace detail {
        felspar::coro::task<void>
                update(felspar::io::warden &,
                       std::span<time *>,
                       std::chrono::nanoseconds);
    }


    /// ## Time tracking performance counter
    /**
     * The time tracking is safe for reads and writes from all threads.
     *
     * Loading adds the saved time to any time already present in the tracker.
     */
    class time final : public performance {
        std::atomic<std::int64_t> ns{};


      public:
        static constexpr std::string_view box{"_p:t:time"};


        time(std::string_view const n) : performance{n} {}
        time(std::string_view const n, std::chrono::nanoseconds const t)
        : performance{n}, ns{t.count()} {}


        auto value() const noexcept {
            return std::chrono::nanoseconds{ns.load()};
        }


        /// ### Changing the value
        void operator+=(std::chrono::nanoseconds const t) noexcept {
            ns += t.count();
        }
        void operator-=(std::chrono::nanoseconds const t) noexcept {
            ns -= t.count();
        }

        /// #### Set to a specific value
        void value(std::chrono::nanoseconds const v) noexcept {
            ns.store(v.count());
        }


        /// ## Track wall time
        /**
         * This is an RAII type designed to track real time into a `time`
         * performance counter. It will add time to the counter using the
         * provided interval.
         */
        template<std::size_t N>
        class track final {
            felspar::io::warden &ward;
            std::array<telemetry::time *, N> times;
            std::chrono::nanoseconds interval = std::chrono::milliseconds{500};


          public:
            /// ### Construct
            /**
             * Pass either a single `time` that you wish to track, or pass an
             * `array` of them to have them all update.
             */
            template<typename... Times>
            track(felspar::io::warden &w, Times &...p)
            : ward{w}, times{&p...} {}
            ~track() { add_duration(); }

            track(track const &) = delete;
            track(track &&) = delete;
            track &operator=(track const &) = delete;
            track &operator=(track &&) = delete;


          private:
            std::chrono::steady_clock::time_point now =
                    std::chrono::steady_clock::now();
            felspar::coro::eager<> updater{update()};

            felspar::coro::task<void> update() {
                while (true) {
                    co_await ward.sleep(interval);
                    add_duration();
                }
            }
            void add_duration() {
                auto const last =
                        std::exchange(now, std::chrono::steady_clock::now());
                auto const duration = now - last;
                for (auto *t : times) { *t += duration; }
            }
        };
        template<typename... Times>
        track(felspar::io::warden &, Times &...) -> track<sizeof...(Times)>;


      private:
        bool save(serialise::save_buffer &) const override;
        bool load(measurements &) override;
    };


}
