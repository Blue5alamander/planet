#pragma once


#include <planet/serialise/exceptions.hpp>
#include <planet/serialise/forward.hpp>
#include <planet/serialise/marker.hpp>

#include <felspar/memory/accumulation_buffer.hpp>
#include <felspar/parse/insert.native.hpp>
#include <felspar/parse/insert.be.hpp>

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


}
