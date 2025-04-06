#pragma once


#include <planet/serialise.hpp>
#include <planet/telemetry/forward.hpp>
#include <planet/telemetry/performance.hpp>

#include <map>
#include <mutex>
#include <set>


namespace planet::telemetry {


    /// ## Table
    /**
     * Stores values against an ordered key.
     */

    /// ### Common table code
    inline std::string_view constexpr table_box_name{"_p:t:table"};


    /// ### Table including additional data
    template<typename Key, typename Value, typename Compare>
    class table final : public performance {
        std::map<Key, Value, Compare> content;
        std::size_t max_entries;


      public:
        static std::string_view constexpr box = table_box_name;


        using key_type = Key;
        using value_type = Value;
        using comparison_type = Compare;


        table(std::string_view const n, std::size_t const max_entries)
        : performance{n}, max_entries{max_entries} {}


        /// #### Queries
        std::size_t size() const noexcept {
            std::scoped_lock _{mutex};
            return content.size();
        }
        auto copy_content() const {
            std::scoped_lock _{mutex};
            return content;
        }


        /// #### Record a new value
        bool add_reading(key_type const &k, value_type const &v) {
            std::scoped_lock _{mutex};
            if (content.size() < max_entries) {
                content[k] = v;
            } else {
                auto const last_iter = --content.end();
                if (comparison_type{}(k, *last_iter)) {
                    content.erase(last_iter);
                    content[k];
                }
            }
        }


      private:
        mutable std::mutex mutex;


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
            std::map<Key, Value, Compare> c;
            if (load_performance_measurement(pd, name(), box, c)) {
                for (auto const &[k, v] : c) { add_reading(k, v); }
                return true;
            } else {
                return false;
            }
        }
    };


    /// ### Table without additional data
    template<typename Key, typename Compare>
    class table<Key, void, Compare> final : public performance {
        std::set<Key, Compare> content;
        std::size_t max_entries;


      public:
        static std::string_view constexpr box = table_box_name;


        using key_type = Key;
        using comparison_type = Compare;


        table(std::string_view const n, std::size_t const max_entries)
        : performance{n}, max_entries{max_entries} {}


        /// #### Queries
        std::size_t size() const noexcept {
            std::scoped_lock _{mutex};
            return content.size();
        }
        auto copy_content() const {
            std::scoped_lock _{mutex};
            return content;
        }


        /// #### Record a new value
        bool add_reading(key_type const &k) {
            std::scoped_lock _{mutex};
            if (content.size() < max_entries) {
                content.insert(k);
                return true;
            } else {
                auto const last_iter = --content.end();
                if (comparison_type{}(k, *last_iter)) {
                    content.erase(last_iter);
                    content.insert(k);
                    return true;
                }
            }
            return false;
        }


      private:
        mutable std::mutex mutex;


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
            std::set<Key, Compare> c;
            if (load_performance_measurement(pd, name(), box, c)) {
                for (auto const k : c) { add_reading(k); }
                return true;
            } else {
                return false;
            }
        }
    };


}
