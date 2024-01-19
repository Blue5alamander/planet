#pragma once


#include <string_view>


namespace planet::events {


    /// ## Back message
    struct back {
        static constexpr std::string_view box{"_p:e:back"};
    };


}
