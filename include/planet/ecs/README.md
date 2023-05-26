# ECS

* [`planet/ecs/component_proxy.hpp`](./component_proxy.hpp) -- A smart pointer which holds a reference to the component storage and an `entity_id` and is able to provide the component on demand.
* [`planet/ecs/entities.hpp`](./entities.hpp) -- Manages the entities within the system and holds references to all component storages. Games will not use this type directly.
* [`planet/ecs/entity.hpp`](./entity.hpp) -- An entity.
* [`planet/ecs/entity_id.hpp`](./entity_id.hpp) -- A strong reference counted identifier for an entity.
* [`planet/ecs/entity_lookup`](./entity_lookup.hpp) -- An abstract type used by component storage to find the entities. This is an internal implementation detail.
* [`planet/ecs/forward.hpp`](./forward.hpp) -- Forward references to ECS types.
* [`planet/ecs/storage.hpp`](./storage.hpp) -- Component storage. Games should group components into individual storages.
* [`planet/ecs/storage_base`](./storage_base.hpp) -- Base class for component storages. An internal implementation detail.
* [`planet/ecs/type_index.hpp`](./type_index.hpp) -- Various internal type look-up functions needed to manage components.
* [`planet/ecs/weak_entity_id.hpp`](./weak_entity_id.hpp) -- A weak reference counted identifier for an entity. It does not keep the entity alive.
