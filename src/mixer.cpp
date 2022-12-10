#include <planet/audio/mixer.hpp>


planet::audio::mix2pipe::mix2pipe() : thread{&mix2pipe::mix_thread, this} {}


planet::audio::mix2pipe::~mix2pipe() { thread.join(); }


void planet::audio::mix2pipe::mix_thread() { mixer_type mixer; }
