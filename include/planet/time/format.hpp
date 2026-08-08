#pragma once


#include <chrono>
#include <string>


namespace planet::time {


    /// ## Human readable duration formatting


    /**
     * Produce a short human readable string for a duration:
     *
     * - up to 999ns the nanosecond count is shown verbatim (`999ns`);
     * - from 1µs up to 100s the value is rounded to three significant figures
     *   with the largest fitting unit — `1.45ms`, `14.5ms`, `145ms`, `1.45s`;
     * - from 100s on it is broken down as `1d2h3m45s`, with the day count
     *   growing without bound.
     */
    std::string display_string(std::chrono::nanoseconds);


}
