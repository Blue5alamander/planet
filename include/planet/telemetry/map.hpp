#pragma once


#include <planet/serialise.hpp>
#include <planet/telemetry/performance.hpp>

#include <map>
#include <mutex>


namespace planet::telemetry {


    /// ## Map
    /**
     * Stores values against an ordered key with in-place update capabilities.
     * Unlike `table`, which only inserts new values, `map` allows updating
     * existing values in-place via a callback function.
     *
     * When loading it will merge the loaded data into the existing container,
     * but drop any key that already exists.
     */
    template<typename Key, typename Value>
    class map final : public performance {
        std::map<Key, Value> content;


      public:
        static std::string_view constexpr box = {"_p:t:map"};


        using key_type = Key;
        using value_type = Value;


        map(std::string_view const n,
            std::source_location loc = std::source_location::current())
        : performance{n, loc} {}


        /// #### Queries
        std::size_t size() const noexcept {
            std::scoped_lock _{mutex};
            return content.size();
        }
        auto copy_content() const {
            std::scoped_lock _{mutex};
            return content;
        }


        /// #### Update a value
        template<typename Lambda>
        void update(key_type const &k, value_type initial, Lambda &&fn)
        /**
         * If the key is absent, it is inserted with `initial`. If present,
         * `fn` is called with a reference to the existing value to modify it
         * in-place.
         */
        {
            std::scoped_lock lock{mutex};
            do_update(k, std::move(initial), std::forward<Lambda>(fn), lock);
        }


      private:
        mutable std::mutex mutex;


        void do_update(
                key_type const &k,
                value_type initial,
                auto &&fn,
                std::scoped_lock<std::mutex> &) {
            auto [it, inserted] = content.try_emplace(k, std::move(initial));
            if (not inserted) { fn(it->second); }
        }


        bool save(serialise::save_buffer &sb) const override {
            std::scoped_lock _{mutex};
            if (content.size()) {
                sb.save_box(box, name(), content);
                return true;
            } else {
                return false;
            }
        }
        bool load(measurements &pd) override {
            std::scoped_lock lock{mutex};
            std::map<Key, Value> c;
            if (load_performance_measurement(pd, name(), box, c)) {
                for (auto const &[k, v] : c) {
                    do_update(k, v, [](auto &) {}, lock);
                }
                return true;
            } else {
                return false;
            }
        }
    };


}
