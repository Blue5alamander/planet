# Planet

**A C++ game engine**

Planet started as just a 2D hex and square grid infinite map implementation. Both make use of a chunking system that aggregates the allocation of larger parts of the map which are created lazily as required by the game.


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
