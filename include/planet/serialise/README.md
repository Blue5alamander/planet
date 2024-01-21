# Serialisation

Serialisation allows game state to be sent over the network or saved to a file. All serialisation is binary and is comprised of a sequence of nested boxes.

Boxes always start with a single byte marker.

* [`plane/serialise/affine.hpp`](./affine.hpp) -- Serialisation of various [affine](../affine/) data types.
* [`planet/serialise/base_types.hpp`](./base_types.hpp) -- Serialisation implementations for various simple types.
* [`plane/serialise/chrono.hpp`](./chrono.hpp) -- Serialisation of `std::chrono` data types.
* [`plane/serialise/collections.hpp`](./collections.hpp) -- Serialisation of `std::` collection data types.
* [`planet/serialise/events.hpp`](./events.hpp) -- Serialisation implementations for the event types.
* [`planet/serialise/exceptions.hpp`](./exceptions.hpp) -- Exceptions that can be throw during serialisation and de-serialisation.
* [`planet/serialise/felspar.hpp`](./felspar.hpp) -- Serialisation for containers in various [Felspar libraries](https://felspar.com/).
* [`planet/serialise/forward.hpp`](./forward.hpp) -- Forward declarations.
* [`planet/serialise/load_buffer.hpp`](./load_buffer.hpp) -- A view into underlying memory that contains the binary serialisation data.
* [`planet/serialise/map.hpp`](./map.hpp) -- Serialisation implementations for the types in the `planet::map` and `planet::hexmap` name spaces.
* [`planet/serialise/marker.hpp`](./marker.hpp) -- The `marker` enumeration which controls the low-level type of a data member.
* [`planet/serialise/muxing.hpp`](./muxing.hpp) -- Sending binary data over a connection.
* [`planet/serialise/save_buffer.hpp`](./save_buffer.hpp) -- A buffer that will receive the serialisation bytes.
* [`planet/serialise/string.hpp`](./string.hpp) -- Serialisation of `std` string types.

There is also the convenience header [`planet/serialise.hpp`](../serialise.hpp) which includes everything.


## Planet's boxes

Box names starting with an underscore (`_`) are reserved for use by Planet.
