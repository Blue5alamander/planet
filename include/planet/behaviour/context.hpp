#pragma once


#include <planet/behaviour/parameter.hpp>
#include <planet/behaviour/type.hpp>

#include <felspar/memory/pmr.hpp>

#include <unordered_map>


namespace planet::behaviour {


    /// ## Behaviour context
    class context_base : public felspar::pmr::memory_resource {
        struct value {
            behaviour::type type;
            void const *cptr;
            void *mptr;
        };
        using map_type = std::unordered_map<char const *, value>;


        map_type map;


      public:
        /// ### Look up a parameter by key and return the argument
        template<typename T>
        T &look_up(parameter<T> const &p) {
            auto &mp = map.at(p.type.id->name());
            /// TODO Check stored type matches
            /// TODO Add checks that `cptr`/`mptr` contain valid values
            if (parameter<T>::is_constant() and mp.cptr) {
                return *reinterpret_cast<T *>(mp.cptr);
            } else {
                return *reinterpret_cast<T *>(mp.mptr);
            }
        }


        /// ### Register a variable by parameter instance
        template<typename T>
        auto operator[](parameter<T> const &p) {
            struct assign {
                parameter<T> const &p;
                map_type &map;
                T &operator=(T &t) {
                    if constexpr (parameter<T>::is_constant()) {
                        map[p.type.id->name()] = {
                                .type = p.type, .cptr = &t, .mptr = nullptr};
                    } else {
                        map[p.type.id->name()] = {
                                .type = p.type, .cptr = nullptr, .mptr = &t};
                    }
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
