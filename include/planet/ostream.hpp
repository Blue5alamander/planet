#pragma once


#include <planet/affine2d.hpp>
#include <planet/hexmap.hpp>

#include <ostream>


namespace planet {


    inline std::ostream &operator<<(std::ostream &os, point2d const p) {
        return os << '(' << p.x() << ", " << p.y() << ')';
    }


    namespace map {
        inline std::ostream &operator<<(std::ostream &os, coordinates const p) {
            return os << '(' << p.column() << ", " << p.row() << ')';
        }
    }


    namespace hexmap {
        inline std::ostream &operator<<(std::ostream &os, coordinates const p) {
            return os << '(' << p.column() << ", " << p.row() << ')';
        }
    }


}
