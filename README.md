# Planet

**A C++ game engine**

[![Documentation](https://badgen.net/static/docs/blue5alamander.com)](https://blue5alamander.com/open-source/planet/)
[![GitHub](https://badgen.net/badge/Github/planet/green?icon=github)](https://github.com/Blue5alamander/planet/)
[![License](https://badgen.net/github/license/Blue5alamander/planet)](https://github.com/Blue5alamander/planet/blob/main/LICENSE_1_0.txt)
[![Discord](https://badgen.net/badge/icon/discord?icon=discord&label)](https://discord.gg/tKSabUa52v)

Planet started as just a [2D hex](https://blue5alamander.com/open-source/planet/include/planet/map/hex.hpp) and [square grid](https://blue5alamander.com/open-source/planet/include/planet/map/square.hpp) infinite map implementation. Both make use of a chunking system that aggregates the allocation of larger parts of the map which are created lazily as required by the game.

Now it consists of several libraries:

* `planet` (this library) -- General game code. Includes [2d and 3d affine transforms](https://blue5alamander.com/open-source/planet/include/planet/affine/), [sound](https://blue5alamander.com/open-source/planet/include/planet/audio/), [UI](https://blue5alamander.com/open-source/planet/include/planet/ui/), [serialisation](https://blue5alamander.com/open-source/planet/include/planet/serialise/), [an ECS](https://blue5alamander.com/open-source/planet/include/planet/ecs/), and more.
* [`planet-sdl`](https://blue5alamander.com/open-source/planet-sdl/) -- Wrapper for use of SDL2 C library with the rest of the Planet engine.
* [`planet-vk`](https://blue5alamander.com/open-source/planet-vk/) -- Wrappers for the Vulkan C library with higher level engine code.
* [`planet-android`](git@github.com:Blue5alamander/planet-android.git) -- Android specific code needed to interface between Planet, SDL2 and the platform.


## Building and running the library and examples


```bash
git clone --recursive git@github.com:KayEss/planet.git
cd planet
mkdir build.tmp
cd build.tmp
cmake ..
make
./examples/snake
```


## Modules

An incomplete list of the available modules are:

* [Behaviours](./include/behaviour/) -- unstable
* [Serialisation](./include/serialise/) -- stable
