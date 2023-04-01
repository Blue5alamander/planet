#pragma once


#include <planet/ui/constrained.hpp>


namespace planet::ui {


    /// ## Layout element
    template<typename T>
    class element {
        constrained2d<T> size;

      public:
        using constrained_type = constrained2d<T>;

        explicit element(constrained_type const &s) : size{s} {}
    };


}
