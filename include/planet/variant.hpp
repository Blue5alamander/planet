#pragma once


#include <variant>


namespace planet {


    namespace detail {
        template<typename... Os>
        struct mixed : private Os... {
            mixed(Os... lambdas) : Os{std::move(lambdas)}... {}
            using Os::operator()...;
        };
    }


    template<typename V, typename... Os>
    auto visit(V &&v, Os... lambdas) {
        return std::visit(detail::mixed{lambdas...}, std::forward<V>(v));
    }


}
