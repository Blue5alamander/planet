#include <planet/audio/music.hpp>

#include <felspar/memory/accumulation_buffer.hpp>

#include <iostream>


planet::audio::stereo_generator planet::audio::music::output() {
    felspar::memory::accumulation_buffer<float> output{
            default_buffer_samples * 50};
    std::optional<planet::audio::stereo_generator> playing;
    while (true) {
        if (clear_flag.load(std::memory_order_relaxed)) {
            clear_flag.store(false, std::memory_order_relaxed);
            std::scoped_lock _{mtx};
            queue.clear();
        }
        if (not playing) {
            std::scoped_lock _{mtx};
            if (queue.size()) { playing = queue.front().start(); }
        }
        if (playing) {
            for (auto block : *playing) {
                if (clear_flag.load(std::memory_order_relaxed)) {
                    /// TODO Micro fade block
                    // co_yield block;
                    break;
                } else {
                    co_yield block;
                }
            }
        } else {
            output.ensure_length(
                    default_buffer_samples * stereo_buffer::channels);
            co_yield output.first(
                    default_buffer_samples * stereo_buffer::channels);
        }
    }
}


felspar::coro::task<void>
        planet::audio::music::clear(felspar::io::warden &ward) {
    std::cout << "Clearing music play queue\n";
    clear_flag.store(true, std::memory_order_relaxed);
    while (clear_flag.load(std::memory_order_relaxed)) {
        co_await ward.sleep(50ms);
    }
    std::cout << "Music queue cleared\n";
}


void planet::audio::music::enqueue(start_tune_function tn) {
    std::scoped_lock _{mtx};
    queue.push_back({std::move(tn)});
}


void planet::audio::music::set_volume(dB_gain const dB) {
    master = dB;
    master_gain.set(static_cast<linear_gain>(master));
}
