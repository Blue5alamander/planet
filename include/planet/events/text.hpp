#pragma once


#include <chrono>
#include <string>


namespace planet::events {


    /// ## Typed text
    /**
     * The characters the user has actually typed, as UTF-8. This is a
     * different thing to `events::key`, which reports the physical keys: a
     * `key` says which key went down, a `text` says what that produced, and
     * the two only line up until a keyboard layout, a dead key or an input
     * method gets involved. A widget that wants what was typed consumes this
     * queue rather than trying to map scan codes to characters itself.
     *
     * One event can carry more than a single codepoint -- a paste, or an input
     * method committing a composition, arrives whole -- so the payload is a
     * string rather than a character.
     */
    struct text final {
        std::string utf8;
        std::chrono::steady_clock::time_point timestamp =
                std::chrono::steady_clock::now();
    };


}
