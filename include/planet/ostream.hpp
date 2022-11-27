#pragma once


#include <planet/affine2d.hpp>
#include <planet/hexmap.hpp>

#include <ostream>


namespace planet {


    namespace affine {
        inline std::ostream &operator<<(std::ostream &os, point2d const p) {
            return os << '(' << p.x() << ", " << p.y() << ')';
        }
        inline std::ostream &operator<<(std::ostream &os, extents2d const e) {
            return os << e.width << "×" << e.height;
        }
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
