#pragma once


#include <felspar/parse/concepts.hpp>
#include <felspar/memory/shared_buffer.hpp>

#include <span>
#include <string_view>


namespace planet::serialise {


    struct box;
    class load_buffer;
    class save_buffer;


    using shared_bytes = felspar::memory::shared_buffer<std::byte>;
    using shared_byte_view = shared_bytes::view_type;


    template<typename S>
    S load_type(load_buffer &);


}
