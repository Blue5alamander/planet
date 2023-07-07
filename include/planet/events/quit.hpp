#pragma once


#include <string_view>


namespace planet::events {


    /// ## Quit message
    struct quit {
        static constexpr std::string_view box{"_p:e:quit"};
    };


}
