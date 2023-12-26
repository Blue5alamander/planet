#pragma once


namespace planet::ui {


    template<typename D>
    concept drawable = requires(D d) {
        { d.draw() };
    };


}
