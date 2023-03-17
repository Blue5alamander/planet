#pragma once


#include <planet/behaviour/key.hpp>
#include <planet/behaviour/parameter.hpp>

#include <felspar/memory/pmr.hpp>

#include <unordered_map>


namespace planet::behaviour {


    /// ## Behaviour context
    class context : public felspar::pmr::memory_resource {
        std::unordered_map<key::id_type, void *> map;

      public:
        /// ### Look up a parameter by key and return the argument
        template<typename T>
        T &look_up(parameter<T> const &p) {
            /// TODO Check stored type matches
            return *reinterpret_cast<T *>(map.at(p.key.id));
        }

        /// ### Register a variable by parameter instance
        template<typename T>
        auto operator[](parameter<T> const &p) {
            struct assign {
                parameter<T> const &p;
                std::unordered_map<key::id_type, void *> &map;
                T &operator=(T &t) {
                    map[p.key.id] = &t;
                    return t;
                }
            };
            return assign{p, map};
        }

        /// ### PMR memory resources
        void *do_allocate(
                std::size_t const bytes, std::size_t const alignment) override {
            return felspar::pmr::new_delete_resource()->allocate(
                    bytes, alignment);
        }
        void do_deallocate(
                void *const p,
                std::size_t const bytes,
                std::size_t const alignment) override {
            return felspar::pmr::new_delete_resource()->deallocate(
                    p, bytes, alignment);
        }
        bool do_is_equal(memory_resource const &other) const noexcept override {
            return this == &other;
        }
    };


}
