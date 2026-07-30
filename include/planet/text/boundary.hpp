#pragma once


#include <cstddef>
#include <string_view>


namespace planet::text {


    /// ## Where a position in text is allowed to be
    /**
     * Everything that asks "what is the thing to the left?" -- backspace, the
     * arrow keys, snapping a caret to a click, either end of a selection --
     * asks it here, so that they cannot disagree about the answer on text no
     * test happens to contain.
     *
     * **A position is a byte offset into the text, and every position handed
     * out sits on a boundary.** Byte offsets are what the callers all want
     * anyway: `std::string::erase` and `insert` take them, and the text before
     * one is a view straight into the buffer with no walk to convert it.
     *
     * The rule here is the **codepoint**: a boundary is any byte that is not a
     * UTF-8 continuation byte. That is enough never to split a character in
     * half, which is all that is promised. It is *not* the unit a reader
     * perceives -- an `e` followed by a combining acute is two boundaries, and
     * a skin-toned or joined emoji several -- and closing that gap means
     * grapheme clusters (UAX #29), a much larger rule. Because that rule lives
     * only in these two functions, adopting it later moves nothing that calls
     * them.
     */


    /// ### Whether a byte continues the character before it
    [[nodiscard]] constexpr bool is_continuation(char const b) noexcept {
        return (static_cast<unsigned char>(b) bitand 0xc0u) == 0x80u;
    }


    /// ### The largest boundary before an offset
    /**
     * `0` at the start of the text, so it never runs off it. An offset landing
     * mid-character comes back as the start of that character, and one past
     * the end is treated as the end.
     */
    [[nodiscard]] constexpr std::size_t previous_boundary(
            std::string_view const text, std::size_t offset) noexcept {
        if (offset > text.size()) { offset = text.size(); }
        while (offset > 0) {
            if (not is_continuation(text[--offset])) { break; }
        }
        return offset;
    }


    /// ### The smallest boundary after an offset
    /**
     * `size()` at the end of the text, so it never runs off it. An offset
     * landing mid-character comes back as the start of the character after it.
     */
    [[nodiscard]] constexpr std::size_t next_boundary(
            std::string_view const text, std::size_t offset) noexcept {
        if (offset >= text.size()) { return text.size(); }
        while (++offset < text.size() and is_continuation(text[offset])) {}
        return offset;
    }


}
