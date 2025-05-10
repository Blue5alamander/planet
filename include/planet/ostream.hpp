#pragma once


#include <planet/affine2d.hpp>
#include <planet/affine/matrix3d.hpp>
#include <planet/map/hex.hpp>

#include <ostream>


namespace planet {


    namespace affine {
        inline std::ostream &operator<<(std::ostream &os, extents2d const e) {
            return os << e.width << "×" << e.height;
        }
        inline std::ostream &operator<<(std::ostream &os, matrix3d const &m) {
            return os << "{{" << m[{0, 0}] << ", " << m[{1, 0}] << ", "
                      << m[{2, 0}] << ", " << m[{3, 0}] << "}\n {" << m[{0, 1}]
                      << ", " << m[{1, 1}] << ", " << m[{2, 1}] << ", "
                      << m[{3, 1}] << "}\n {" << m[{0, 2}] << ", " << m[{1, 2}]
                      << ", " << m[{2, 2}] << ", " << m[{3, 2}] << "}\n {"
                      << m[{0, 3}] << ", " << m[{1, 3}] << ", " << m[{2, 3}]
                      << ", " << m[{3, 3}] << "}}";
        }
        inline std::ostream &operator<<(std::ostream &os, point2d const p) {
            return os << '(' << p.x() << ", " << p.y() << ')';
        }
        inline std::ostream &operator<<(std::ostream &os, rectangle2d const r) {
            return os << r.extents << '@' << r.top_left;
        }
    }


    namespace hexmap {
        inline std::ostream &operator<<(std::ostream &os, coordinates const p) {
            return os << '(' << p.column() << ", " << p.row() << ')';
        }
    }


    namespace map {
        inline std::ostream &operator<<(std::ostream &os, coordinates const p) {
            return os << '(' << p.column() << ", " << p.row() << ')';
        }
    }


}
