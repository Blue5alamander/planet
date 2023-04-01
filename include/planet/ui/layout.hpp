#pragma once


#include <planet/ui/element.hpp>


namespace planet::ui {


    /// ## A layout
    template<typename T, typename C = std::vector<element<T>>>
    class layout {
        C elements;

      public:
        using value_type = T;

        layout() = default;
    };


}
