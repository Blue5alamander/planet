#pragma once


#include <planet/serialise/exceptions.hpp>
#include <planet/serialise/forward.hpp>
#include <planet/serialise/marker.hpp>

#include <felspar/memory/shared_vector.hpp>
#include <felspar/parse.binary.hpp>

#include <string_view>


namespace planet::serialise {


    /// ## Serialisation save buffer
    class save_buffer {
        felspar::memory::shared_bytes buffer;
        std::size_t written = {};

      public:
        save_buffer();

        template<typename... Args>
        save_buffer &save_box(std::string_view name, Args &&...args) {
            if (name.empty() or name.size() >= 0x80) {
                throw box_name_length(std::string{name});
            }
            append(static_cast<std::uint8_t>(name.size()));
            append(std::as_bytes(std::span{name.data(), name.size()}));
            append(std::uint8_t(1));
            auto const size_offset = allocate_offset(sizeof(std::uint64_t));
            (save(*this, std::forward<Args>(args)), ...);
            auto const length = written - size_offset - sizeof(std::uint64_t);
            felspar::parse::binary::unchecked_insert(
                    std::span<std::byte, sizeof(std::uint64_t)>{
                            buffer.data() + size_offset, sizeof(std::uint64_t)},
                    std::uint64_t(length));
            return *this;
        }

        void append(marker const m) { append(static_cast<std::uint8_t>(m)); }
        void append_size_t(std::size_t);
        void append(std::string_view);
        void append(std::span<std::byte const>);
        void append(felspar::parse::concepts::integral auto v) {
            felspar::parse::binary::unchecked_insert(allocate_for(v), v);
        }

        felspar::memory::shared_bytes complete();

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
