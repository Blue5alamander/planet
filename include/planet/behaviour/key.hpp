#pragma once


namespace planet::behaviour {


    /// ## Context value look-up key
    struct key {
        using id_type = char const *;
        id_type id;

        constexpr explicit key(id_type const i) : id{i} {}
    };


}
