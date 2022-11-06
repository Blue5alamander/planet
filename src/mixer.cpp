#include <planet/audio/mixer.hpp>


planet::audio::mixer::mixer() : thread{&mixer::mix_thread, this} {}


planet::audio::mixer::~mixer() { thread.join(); }


void planet::audio::mixer::mix_thread() {}
