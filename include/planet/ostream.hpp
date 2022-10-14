#include <planet/hexmap.hpp>

#include <ostream>


namespace planet {


    namespace map {
        std::ostream &operator<<(std::ostream &os, coordinates const p) {
            return os << '(' << p.column() << ", " << p.row() << ')';
        }
    }


    namespace hexmap {
        std::ostream &operator<<(std::ostream &os, coordinates const p) {
            return os << '(' << p.column() << ", " << p.row() << ')';
        }
    }


}
