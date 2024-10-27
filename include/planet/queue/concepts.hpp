#pragma once


#include <utility>


namespace planet::queue {


    template<typename Q>
    concept push_value_queue = requires(Q q) {
        typename Q::value_type;
        { q.push(std::declval<Q::value_type>()) };
    };
    template<typename Q>
    concept push_void_queue = requires(Q q) {
        { q.push() };
    };


}
