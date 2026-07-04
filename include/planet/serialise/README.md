# serialise

Serialisation allows game state to be sent over the network or saved to a file. All serialisation is binary and is comprised of a sequence of nested boxes.

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
* [`planet/serialise/variant.hpp`](./variant.hpp) -- Serialisation of `std::variant`, storing the active alternative without an enclosing box. See the [variant tests](../../../src/serialise.variant.tests.cpp) for a full explanation of how to use them.

There is also the convenience header [`planet/serialise.hpp`](../serialise.hpp) which includes everything. You should use this anywhere that you implement `save` or `load` rather than try to work out which set of headers is minimal.


## Planet's boxes

Box names starting with an underscore (`_`) are reserved for use by Planet. Typically `std::` types begin with `_s:` and Planet ones start with `_p:`.


## Migration

Save files outlive the code that wrote them, so `load` implementations need to cope with boxes written by older versions of the code. Every box carries its name, a version number (which defaults to 1 when saving) and the size of its content, and these support the migration patterns below.


### Adding fields

Append new fields to the end of the box. The save code always writes them, and the load code reads them only when there is still data left in the box:

```cpp
void save(planet::serialise::save_buffer &sb, part const &p) {
    sb.save_box(p.box, p.name, p.start, p.duration, p.gain, p.from);
}
void load(planet::serialise::box &b, part &p) {
    b.lambda(p.box, [&]() {
        b.fields(p.name, p.start, p.duration);
        if (b.content.empty()) { return; }
        b.fields(p.gain);
        if (b.content.empty()) { return; }
        b.fields(p.from);
    });
}
```

A field skipped this way keeps whatever value it had before `load` was called, so members added later must have sensible defaults.

For changes that can't be expressed by appending fields, bump the box version when saving and switch on `version` when loading. Always throw for versions that aren't understood:

```cpp
void save(planet::serialise::save_buffer &sb, thing const &t) {
    sb.save_box(2, t.box, t.height, t.width);
}
void load(planet::serialise::box &b, thing &t) {
    b.lambda(t.box, [&]() {
        if (b.version == 2) {
            b.fields(t.height, t.width);
        } else if (b.version == 1) {
            b.fields(t.width, t.height);
        } else {
            b.throw_unsupported_version(2);
        }
    });
}
```

The `std::chrono::duration` support in [`planet/serialise/chrono.hpp`](./chrono.hpp) is a real example of this pattern.


### Removing fields

Stop saving the field, but the load code must still consume it from older saves or the box will fail its end-of-load empty check. Bump the version so the load knows whether the field is present, then load the old field into a discarded local:

```cpp
void load(planet::serialise::box &b, thing &t) {
    b.lambda(t.box, [&]() {
        b.fields(t.kept);
        if (b.version == 1) {
            float removed = {};
            b.fields(removed);
        }
        b.fields(t.also_kept);
    });
}
```

Loading into a discarded local only works while the field's type still exists. When the removed field represents a type that has also been removed from the system there is nothing left to load into, so use `skip_box` from [`planet/serialise/load_buffer.hpp`](./load_buffer.hpp) instead. It only needs the name the removed type's box was saved under — it consumes the box, checks the name and discards the content. Loading a `skip_box` is `constexpr` safe as the load never writes to it, so the instance can be declared `static` and `constexpr`:

```cpp
static planet::serialise::skip_box constexpr removed{"old_field"};
b.fields(removed);
```


### Renaming a box

To rename a box just change the name the save code uses — here that's the `tempo::box` constant, so the save itself needs no changes and new saves are always written under the current name. Only the load has to know about old names so that existing save files still work. `check_name_or_throw`, `named` and `lambda` on `box`, as well as `load_box` on `load_buffer`, all have overloads taking a `std::span<std::string_view const>` which accept a box with any of the provided names:

```cpp
using namespace std::literals;

void save(planet::serialise::save_buffer &sb, tempo const &t) {
    sb.save_box(t.box, t.count, t.beat, t.bpm);
}
void load(planet::serialise::box &b, tempo &t) {
    static std::array constexpr tempo_names{tempo::box, "bpm"sv};
    b.named(tempo_names, t.count, t.beat, t.bpm);
}
```

The array lists the current name followed by every name the box has been saved under before — after the rename the old name (`"bpm"` here) survives only as a literal in the load. Old saves load under their old name and are written back out under the current one, so files migrate the next time they're saved.
