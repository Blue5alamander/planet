#pragma once


#include <utility>


namespace planet {


    /// ## 'by_index' -- Iterate over array/vector indexes
    template<typename Lambda>
    constexpr inline auto by_index(
            std::size_t const start_index,
            std::size_t const max_index,
            Lambda &&lambda) {
        for (std::size_t index{start_index}; index < max_index; ++index) {
            lambda(index);
        }
    }
    template<typename Lambda>
    constexpr inline auto
            by_index(std::size_t const max_index, Lambda &&lambda) {
        return by_index({}, max_index, std::forward<Lambda>(lambda));
    }


}
