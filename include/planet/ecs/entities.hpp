#pragma once


#include <planet/ecs/entity_lookup.hpp>
#include <planet/ecs/storage_base.hpp>

#include <tuple>


namespace planet::ecs {


    /// ## Entities
    /**
     * The game should create a single instance of this type giving it the
     * individual entity storages so that they can be aggregated and ensure that
     * all entities have access to all components so that systems can iterate
     * them properly.
     */
    template<typename... Storages>
    class entities final :
    public entity_lookup,
            public base_entities<Storages>... {
        std::vector<ecs::entity> e_slots;
        std::tuple<Storages &...> stores;
        using indexes = std::index_sequence_for<Storages...>;

      public:
        entities(Storages &...s) : stores{s...} {
            [this]<std::size_t... Is>(std::index_sequence<Is...>) {
                (std::get<Is>(stores).set_entities_storage_index(this, Is),
                 ...);
            }
            (indexes{});
        }

        /// Not movable, not copyable
        entities(entities &&) = delete;
        entities(entities const &) = delete;
        entities &operator=(entities &&) = delete;
        entities &operator=(entities const &) = delete;


        /// The number of entity storages that are in use
        static constexpr std::size_t store_count = sizeof...(Storages);

        /// Create and return new entity
        entity_id create() override {
            /// TODO Search for a free slot in e_slots
            e_slots.emplace_back(store_count);

            [this]<std::size_t... Is>(std::index_sequence<Is...>) {
                (std::get<Is>(stores).emplace_back(), ...);
            }
            (indexes{});

            return {this, e_slots.size() - 1};
        }

        ecs::entity &entity(std::size_t const eid) override {
            return e_slots.at(eid);
        }
    };


}
