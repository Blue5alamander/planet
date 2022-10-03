# Planet

An implementation of containers for 2D square and hex maps, of the sort suitable for simple games.

It contains implementations for both square and hex grids. Both make use of a chunking system that aggregates the allocation of larger parts of the map which are created lazily as required by the game.

There is also some help for implementing the control mechanism between a player or AI and the actual game.


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
