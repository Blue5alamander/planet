#pragma once


#include <planet/serialise/exceptions.hpp>
#include <planet/serialise/forward.hpp>
#include <planet/serialise/marker.hpp>

#include <felspar/memory/accumulation_buffer.hpp>
#include <felspar/parse/insert.native.hpp>
#include <felspar/parse/insert.be.hpp>

#include <filesystem>
#include <fstream>
#include <optional>
#include <string_view>


namespace planet::serialise {


    /// ## Serialisation save buffer
    class save_buffer {
        felspar::memory::accumulation_buffer<std::byte> buffer;
        std::size_t written = {};

      public:
        using accumulation_buffer =
                felspar::memory::accumulation_buffer<std::byte>;
        using shared_bytes = accumulation_buffer::buffer_type;

        save_buffer();

        template<typename Lambda>
        save_buffer &save_box_lambda(
                std::uint8_t const version,
                std::string_view const name,
                Lambda lambda) {
            if (name.empty() or name.size() >= 0x80) {
                throw box_name_length(std::string{name});
            }
            append(static_cast<std::uint8_t>(name.size()));
            append(std::as_bytes(std::span{name.data(), name.size()}));
            append(version);
            auto const size_offset = allocate_offset(sizeof(std::uint64_t));
            lambda();
            auto const length = written - size_offset - sizeof(std::uint64_t);
            felspar::parse::binary::be::unchecked_insert(
                    std::span<std::byte, sizeof(std::uint64_t)>{
                            buffer.memory().data() + size_offset,
                            sizeof(std::uint64_t)},
                    std::uint64_t(length));
            return *this;
        }
        template<typename Lambda>
        save_buffer &
                save_box_lambda(std::string_view const name, Lambda &&lambda) {
            return save_box_lambda(
                    std::uint8_t{1}, name, std::forward<Lambda>(lambda));
        }
        save_buffer &save_box(
                std::uint8_t const version, std::string_view const name) {
            return save_box_lambda(version, name, []() {});
        }
        template<typename... Args>
        save_buffer &save_box(
                std::uint8_t const version,
                std::string_view const name,
                Args &&...args) {
            return save_box_lambda(version, name, [&, p = this]() {
                (save(*p, std::forward<Args>(args)), ...);
            });
        }
        save_buffer &save_box(std::string_view const name) {
            return save_box_lambda(std::uint8_t{1}, name, []() {});
        }
        template<typename... Args>
        save_buffer &save_box(std::string_view const name, Args &&...args) {
            return save_box_lambda(std::uint8_t{1}, name, [&, p = this]() {
                (save(*p, std::forward<Args>(args)), ...);
            });
        }


        /// ### Saving raw data
        void append(marker const m) { append(static_cast<std::uint8_t>(m)); }
        void append_size_t(std::size_t);
        void append(std::string_view);
        void append(std::span<std::byte const>);
        void append(std::span<char const>);
        void append(std::string const &s) { append(std::string_view{s}); }
        void append(felspar::parse::concepts::numeric auto v) {
            felspar::parse::binary::native::unchecked_insert(
                    allocate_for(v), v);
        }


        /// ### The number of bytes currently in the save
        std::size_t size() const noexcept { return written; }

        /// ### Complete this save and return the save data
        shared_bytes complete();

      private:
        /// Returns a span for the bytes that have been allocated
        std::size_t allocate_offset(std::size_t);
        std::span<std::byte> allocate(std::size_t);
        template<typename T>
        auto allocate_for(T) {
            return std::span<std::byte, sizeof(T)>{
                    allocate(sizeof(T)).data(), sizeof(T)};
        }
    };


    /// ### Save a value into a file
    /**
     * The size of the file the save left behind, or nothing at all if the file
     * did not come out of the save holding the value — the stream took the
     * bytes and the file is the size they were is what makes the difference. An
     * empty return leaves the file however the failed write left it, so a
     * caller with something to lose should save beside the file it is replacing
     * and only put the new one in place once this reports a size.
     *
     * TODO Save into a temporary file first and then atomically rename over
     * the requested filename, so the file is never seen part written.
     */
    template<typename T>
    [[nodiscard]] inline std::optional<std::size_t>
            save(std::filesystem::path const &fn, T const &t) {
        save_buffer sb;
        save(sb, t);
        auto const bytes = sb.complete();
        std::ofstream out{fn, std::ios::binary};
        out.write(reinterpret_cast<char const *>(bytes.data()), bytes.size());
        out.close();
        std::error_code error;
        if (out and std::filesystem::file_size(fn, error) == bytes.size()) {
            return bytes.size();
        } else {
            return {};
        }
    }

    template<typename V1, typename V2, typename... Vs>
    inline auto save(save_buffer &sb, V1 &v1, V2 &v2, Vs &...vs) {
        save(sb, v1);
        save(sb, v2);
        (save(sb, vs), ...);
    }


}
