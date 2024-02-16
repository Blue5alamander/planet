#pragma once


#include <planet/serialise/exceptions.hpp>
#include <planet/serialise/forward.hpp>
#include <planet/serialise/marker.hpp>

#include <felspar/memory/shared_vector.hpp>
#include <felspar/parse/extract.be.hpp>
#include <felspar/parse/extract.le.hpp>
#include <felspar/parse/extract.native.hpp>

#include <filesystem>
#include <fstream>


namespace planet::serialise {


    /// ## Buffer for loading data
    /**
     * The `load_buffer` does not own any memory, it is merely a view over an
     * existing byte buffer that allows for tracking of what has been
     * de-serialised so far with helpers to extract low level data types.
     */
    class load_buffer {
        std::span<std::byte const> buffer;


      public:
        /// ### Construction
        load_buffer() {}
        explicit load_buffer(std::span<std::byte const> b) : buffer{b} {}


        /// ### Queries
        bool empty() const noexcept { return buffer.empty(); }
        auto size() const noexcept { return buffer.size(); }
        auto cmemory() const noexcept { return buffer; }


        /// ### Mutation
        auto split(std::size_t const bytecount) {
            auto const r = buffer.first(bytecount);
            buffer = buffer.subspan(bytecount);
            return r;
        }


        /// ### Data extraction
        auto extract_marker() { return marker{extract<std::uint8_t>()}; }
        std::size_t extract_size_t(
                felspar::source_location const & =
                        felspar::source_location::current());
        template<felspar::parse::concepts::numeric T>
        T
                extract(felspar::source_location const &loc =
                                felspar::source_location::current()) {
            if (buffer.size() < sizeof(T)) {
                throw buffer_not_big_enough{sizeof(T), buffer.size(), loc};
            } else {
                return felspar::parse::binary::native::extract<T>(buffer, loc);
            }
        }
        template<felspar::parse::concepts::numeric T>
        T extract_non_native(
                felspar::source_location const &loc =
                        felspar::source_location::current()) {
            if (buffer.size() < sizeof(T)) {
                throw buffer_not_big_enough{sizeof(T), buffer.size(), loc};
            } else {
                auto const s = std::span<std::byte const, sizeof(T)>{
                        buffer.data(), sizeof(T)};
                auto const v = [&]() {
                    if constexpr (
                            felspar::parse::endian::native
                            == felspar::parse::endian::big) {
                        return felspar::parse::binary::le::unchecked_extract<T>(
                                s);
                    } else {
                        return felspar::parse::binary::be::unchecked_extract<T>(
                                s);
                    }
                }();
                buffer = buffer.subspan(sizeof(T));
                return v;
            }
        }

        /// #### Extract and check a marker
        void check_marker(
                marker const m,
                felspar::source_location const &loc =
                        felspar::source_location::current()) {
            auto const r = extract_marker();
            if (r != m) { throw wrong_marker{m, r, loc}; }
        }


        /// ### Load the next box
        template<typename... Args>
        void load_box(std::string_view, Args &...);
    };


    /**
     * ### Loading boxes
     *
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
        std::uint8_t version = {};
        load_buffer content;

        void check_name_or_throw(
                std::string_view expected,
                felspar::source_location const & =
                        felspar::source_location::current()) const;
        void check_empty_or_throw(
                felspar::source_location const & =
                        felspar::source_location::current()) const;

        /// #### Check the provided name and then load the fields
        template<typename... Args>
        void named(std::string_view const name, Args &&...args) {
            try {
                check_name_or_throw(name);
                (load(content, std::forward<Args>(args)), ...);
                check_empty_or_throw();
            } catch (serialisation_error &e) {
                e.inside_box(name);
                throw;
            }
        }
    };
    void load(load_buffer &, box &);


    template<typename... Args>
    void load_buffer::load_box(std::string_view const name, Args &...args) {
        load_type<box>(*this).named(name, args...);
    }

    inline void load(load_buffer &l, box &b) {
        auto const mark = l.extract_marker();
        if (not is_box_marker(mark)) {
            throw wanted_box{l.cmemory(), mark};
        } else {
            auto const name_bytes = l.split(static_cast<std::uint8_t>(mark));
            b.name = {
                    reinterpret_cast<char const *>(name_bytes.data()),
                    name_bytes.size()};
            b.version = l.extract<std::uint8_t>();
            auto const bytes = l.extract_size_t();
            b.content = load_buffer{l.split(bytes)};
        }
    }


    template<typename S>
    S load_type(load_buffer &v) {
        S s;
        load(v, s);
        return s;
    }
    template<typename S>
    S load_type(box &b) {
        S s;
        load(b, s);
        return s;
    }
    template<typename S>
    S load_type(shared_byte_view v) {
        auto b = load_buffer{v.cmemory()};
        auto s{load_type<S>(b)};
        if (not b.empty()) {
            throw felspar::stdexcept::runtime_error{
                    "There is still data in the buffer after loading the type"};
        }
        return s;
    }


    template<typename T>
    void load(load_buffer &lb, T &t)
        requires(not felspar::parse::concepts::numeric<T>)
    {
        auto b = load_type<box>(lb);
        load(b, t);
    }


    /// ### Load directly from a file
    template<typename T>
    void load(std::filesystem::path const &fn, T &t) {
        std::vector<char> buffer(std::filesystem::file_size(fn));
        std::ifstream{fn, std::ios::binary}.read(buffer.data(), buffer.size());
        load_buffer lb{std::as_bytes(std::span{buffer})};
        load(lb, t);
        if (not lb.empty()) {
            throw felspar::stdexcept::runtime_error{
                    "There is still data in the buffer after loading the type"};
        }
    }


}
