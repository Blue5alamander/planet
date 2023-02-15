#pragma once


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
            append(name);
            append(std::uint8_t(1));
            auto const size_offset = allocate_offset(sizeof(std::size_t));
            (save(*this, std::forward<Args>(args)), ...);
            felspar::parse::binary::unchecked_insert(
                    std::span<std::byte, sizeof(std::size_t)>{
                            buffer.data() + size_offset, sizeof(std::size_t)},
                    written - size_offset - sizeof(std::size_t));
            return *this;
        }

        void append(std::string_view);
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
