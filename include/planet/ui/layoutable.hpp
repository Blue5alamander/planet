#pragma once


#include <planet/ui/reflowable.hpp>


namespace planet::ui {


    template<typename D>
    concept layoutable = requires(D d) {
        { d.draw() };
    };


}
