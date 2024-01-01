#pragma once


#include <concepts>


namespace planet::ui {


    /// ## Widget concepts


    /// ### A type which may be visible or not
    template<typename V>
    concept visibility = requires(V v, V const cv, bool b) {
        { v.visible(b) };
        { cv.is_visible() } -> std::same_as<bool>;
    };


}
