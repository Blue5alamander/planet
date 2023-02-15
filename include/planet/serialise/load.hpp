#pragma once


#include <planet/serialise/forward.hpp>
#include <planet/serialise/marker.hpp>

#include <felspar/memory/shared_vector.hpp>
#include <felspar/parse/extract.hpp>


namespace planet::serialise {


    class load_buffer {
        std::span<std::byte const> buffer;

      public:
        load_buffer() {}
        explicit load_buffer(std::span<std::byte const> b) : buffer{b} {}

        bool empty() const noexcept { return buffer.empty(); }

        auto split(std::size_t const bytecount) {
            auto const r = buffer.first(bytecount);
            buffer = buffer.subspan(bytecount);
            return r;
        }

        template<felspar::parse::concepts::integral T>
        T extract() {
            return felspar::parse::binary::extract<T>(buffer);
        }

        template<typename... Args>
        void load_box(std::string_view, Args &...);
    };


    struct box {
        std::string_view name;
        load_buffer content;

        void check_name_or_throw(std::string_view expected) const;
        void check_empty_or_throw(
                felspar::source_location const & =
                        felspar::source_location::current()) const;

        friend void load(load_buffer &, box &);
    };
    void load(load_buffer &, box &);


    template<typename... Args>
    inline void load_buffer::load_box(std::string_view name, Args &...args) {
        auto b = load_type<box>(*this);
        b.check_name_or_throw(name);
        (load(b.content, args), ...);
        b.check_empty_or_throw();
    }


    template<typename S>
    inline S load_type(load_buffer &v) {
        S s;
        load(v, s);
        return s;
    }
    template<typename S>
    inline S load_type(felspar::memory::shared_byte_view v) {
        auto b = load_buffer{v.cmemory()};
        auto s{load_type<S>(b)};
        if (not b.empty()) {
            throw felspar::stdexcept::runtime_error{
                    "There is still data in the buffer after loading the type"};
        }
        return s;
    }


}
