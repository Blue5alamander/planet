#pragma once


#include <planet/serialise/exceptions.hpp>
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
        auto cmemory() const noexcept { return buffer; }

        auto split(std::size_t const bytecount) {
            auto const r = buffer.first(bytecount);
            buffer = buffer.subspan(bytecount);
            return r;
        }

        auto extract_marker() { return marker{extract<std::uint8_t>()}; }
        std::size_t extract_size_t(
                felspar::source_location const & =
                        felspar::source_location::current());
        template<felspar::parse::concepts::integral T>
        T
                extract(felspar::source_location const &loc =
                                felspar::source_location::current()) {
            if (buffer.size() < sizeof(T)) {
                throw buffer_not_big_enough{sizeof(T), buffer.size(), loc};
            } else {
                return felspar::parse::binary::extract<T>(buffer, loc);
            }
        }

        template<typename... Args>
        void load_box(std::string_view, Args &...);
    };


    /**
     * Loading an instance of a box from a `load_buffer` allows a loader to
     * explicitly deal with various box loading needs. For example:
     *
     * 1. Manual loading of box content.
     * 2. To skip a box.
     *
     * The `load_buffer` instance the box is loaded from will now point to after
     * the box. To load from the content the `content` member should be used.
     */
    struct box {
        std::string_view name;
        load_buffer content;

        void check_name_or_throw(
                std::string_view expected,
                felspar::source_location const & =
                        felspar::source_location::current()) const;
        void check_empty_or_throw(
                felspar::source_location const & =
                        felspar::source_location::current()) const;

        friend void load(load_buffer &, box &);
    };
    void load(load_buffer &, box &);


    template<typename... Args>
    inline void load_buffer::load_box(std::string_view name, Args &...args) {
        try {
            auto b = load_type<box>(*this);
            b.check_name_or_throw(name);
            (load(b.content, args), ...);
            b.check_empty_or_throw();
        } catch (serialisation_error &e) {
            e.inside_box(name);
            throw;
        }
    }

    inline void load(load_buffer &l, box &b) {
        auto const mark = l.extract_marker();
        if (not is_box_marker(mark)) {
            throw felspar::stdexcept::runtime_error{"Wasn't a box marker"};
        } else {
            auto const name_bytes = l.split(static_cast<std::uint8_t>(mark));
            b.name = {
                    reinterpret_cast<char const *>(name_bytes.data()),
                    name_bytes.size()};
            [[maybe_unused]] auto const version = l.extract<std::uint8_t>();
            auto const bytes = l.extract_size_t();
            b.content = load_buffer{l.split(bytes)};
        }
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
