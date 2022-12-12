#pragma once


#include <planet/affine2d.hpp>
#include <planet/map/hex.hpp>

#include <ostream>


namespace planet {


    namespace affine {
        inline std::ostream &operator<<(std::ostream &os, point2d const p) {
            return os << '(' << p.x() << ", " << p.y() << ')';
        }
        inline std::ostream &operator<<(std::ostream &os, extents2d const e) {
            return os << e.width << "×" << e.height;
        }
        inline std::ostream &operator<<(std::ostream &os, rectangle const r) {
            return os << r.extents << '@' << r.top_left;
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
