#pragma once


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
        serialisation_error(std::string, felspar::source_location const &);
        serialisation_error(
                std::string,
                std::span<std::byte const>,
                felspar::source_location const &);

      public:
        /// This happened inside the given box
        void inside_box(std::string_view);
    };


    /// ### Box name too long
    class box_name_length : public serialisation_error {
      public:
        box_name_length(
                std::string_view name,
                felspar::source_location const & =
                        felspar::source_location::current());
    };


    /// ### A box still has data after reading, or the box is not large enough
    class box_not_empty : public serialisation_error {
      public:
        box_not_empty(
                felspar::source_location const & =
                        felspar::source_location::current());
    };
    class buffer_not_big_enough : public serialisation_error {
      public:
        buffer_not_big_enough(
                std::size_t wanted,
                std::size_t got,
                felspar::source_location const & =
                        felspar::source_location::current());
    };


    /// ### Marker errors
    class wanted_boolean : public serialisation_error {
      public:
        wanted_boolean(
                std::span<std::byte const>,
                marker got,
                felspar::source_location const & =
                        felspar::source_location::current());
    };
    class wrong_marker : public serialisation_error {
      public:
        wrong_marker(
                std::span<std::byte const>,
                marker exected,
                marker got,
                felspar::source_location const & =
                        felspar::source_location::current());
    };


}
