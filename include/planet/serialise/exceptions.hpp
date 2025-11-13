#pragma once


#include <planet/serialise/forward.hpp>
#include <planet/serialise/marker.hpp>

#include <felspar/exceptions.hpp>

#include <span>


namespace planet::serialise {


    /// ## Serialisation exceptions


    /// ### Super-class for all serialisation errors
    class serialisation_error : public felspar::stdexcept::runtime_error {
        std::string what_message, hexdump;
        char const *what() const noexcept override;

      protected:
        serialisation_error(std::string, std::source_location const &);
        serialisation_error(
                std::string,
                std::span<std::byte const>,
                std::source_location const &);

      public:
        /// This happened inside the given box
        void inside_box(std::string_view);
    };


    /// ### Box name too long
    class box_name_length : public serialisation_error {
      public:
        box_name_length(
                std::string_view name,
                std::source_location const & = std::source_location::current());
    };


    /// ### Invalid character size
    /**
     * Only UTF8, UTF16 and UTF32 are supported, so all character byte sizes
     * must be either 1, 2 or 4.
     */
    class invalid_charsize : public serialisation_error {
      public:
        invalid_charsize(
                std::size_t,
                std::source_location const & = std::source_location::current());
    };


    /**
     * ### The version number isn't supported
     *
     * Pass it the box and the highest supported version number. This error can
     * also be thrown through a method on the `box` type.
     */
    class unsupported_version_number : public serialisation_error {
      public:
        unsupported_version_number(
                box const &,
                std::uint8_t,
                std::source_location const & = std::source_location::current());
    };


    /// ### A box still has data after reading, or the box is not large enough
    class box_not_empty : public serialisation_error {
      public:
        box_not_empty(
                box const &,
                std::source_location const & = std::source_location::current());
    };
    class buffer_not_big_enough : public serialisation_error {
      public:
        buffer_not_big_enough(
                std::size_t wanted,
                std::size_t got,
                std::source_location const & = std::source_location::current());
    };


    /// ### Marker errors
    class wanted_boolean : public serialisation_error {
      public:
        wanted_boolean(
                std::span<std::byte const>,
                marker got,
                std::source_location const & = std::source_location::current());
    };
    class wanted_box : public serialisation_error {
      public:
        wanted_box(
                std::span<std::byte const>,
                marker got,
                std::source_location const & = std::source_location::current());
    };
    class wrong_marker : public serialisation_error {
      public:
        wrong_marker(
                marker exected,
                marker got,
                std::source_location const & = std::source_location::current());
        wrong_marker(
                std::span<std::byte const>,
                marker exected,
                marker got,
                std::source_location const & = std::source_location::current());
    };


}
