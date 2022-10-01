#include <planet/map.hpp>


namespace planet::hexmap {


    class coordinate {
        map::coordinate pos;
        constexpr coordinate(map::coordinate p) : pos{p} {}

      public:
        constexpr coordinate() noexcept {}
        constexpr coordinate(long x, long y) noexcept
        : pos{x, (y < 0 ? y - 1 : y) / 2} {}

        constexpr long row() const noexcept {
            return (pos.row() * 2) + (pos.column() bitand 1);
        }
        constexpr long column() const noexcept { return pos.column(); }

        constexpr coordinate operator+(coordinate const r) const noexcept {
            return {column() + r.column(), row() + r.row()};
        }

        constexpr auto operator<=>(coordinate const &) const noexcept = default;
    };

    constexpr coordinate east{2, 0}, north_east{1, 1}, north_west{-1, 1},
            west{-2, 0}, south_west{-1, -1}, south_east{1, -1};
    constexpr std::array<coordinate, 6> directions{
            east, north_east, north_west, west, south_west, south_east};


}
