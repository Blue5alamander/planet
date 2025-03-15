# Telemetry

The telemetry module allows for the recording of various statistics about ongoing processes. This can vary from the current number of frames per second the GPU is outputting to game statistics like how many kills the player has made.

* [`plane/telemetry/counter.hpp`](./counter.hpp) -- The `counter` type provides a thread safe way to count events.
* [`plane/telemetry/forward.hpp`](./forward.hpp) -- Forward declarations.
* [`plane/telemetry/id.hpp`](./id.hpp) -- Creates unique names for instances of its sub-classes.
* [`plane/telemetry/minmax.hpp`](./minmax.hpp) -- Stores mimimum and maximums of encountered values. Minimum is not yet implemented.
* [`plane/telemetry/performance.hpp`](./performance.hpp) -- The super-class for all performance counters which provides simplified handling for serialisation.
* [`plane/telemetry/rate.hpp`](./rate.hpp) -- Provides various ways to capture change of values over time or samples taken.
* [`plane/telemetry/time.hpp`](./time.hpp) -- Records a total duration, or how long something has been happening in total.
* [`plane/telemetry/timestamps.hpp`](./timestamps.hpp) -- Records whether a named event has happened, when it last happened and a count for how many times it has happened.


## Use cases for the various types


* `planet::telemetry::counter` -- Player deaths; times a game has been loaded; number of enemies spawned.
* `planet::telemetry::exponential_decay` -- [Number of active widgets on screen](https://blue5alamander.com/open-source/planet/src/baseplate.ui.cpp).
* `planet::telemetry::max` -- Maximum number of active enemies; [number of textures used in a given frame](https://blue5alamander.com/open-source/planet-vk/include/planet/vk/engine/sprite.pipeline.hpp#performance-counters).
* `planet::telemetry::real_time_decay` -- Actual DPS.
* `planet::telemetry::real_time_rate` -- Game FPS.
* `planet::telemetry::time` -- Total play time; total time spent on each game level.
* `planet::telemetry::timestamps` -- Player consents shown; boss kills.
