#include <iostream>
#include <alsa/asoundlib.h>
#include <jack/jack.h>

#include "../src/audio.h"

void alsa_error_handler (const char* file, int line, const char* function, int err, const char* fmt, ...) {
    const auto size = 1024;
    char buffer [size];
    va_list arg;
    va_start (arg, fmt);
    snprintf(buffer, size, fmt, arg);
    va_end (arg);
}

void jack_error_handler (const char* text) {}

int main(int argc, char* argv[]) {
    snd_lib_error_set_handler(alsa_error_handler);
    jack_set_error_function(jack_error_handler);

    portaudio::AutoSystem autoSystem;

    for (const auto& name: device_names()) {
        std::cout << name << std::endl;
    }

    return 0;
}
