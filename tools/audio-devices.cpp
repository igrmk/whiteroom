#include <iostream>
#ifdef __linux__
    #include <alsa/asoundlib.h>
    #include <jack/jack.h>
#endif

#include "../src/audio.h"

#ifdef __linux__
    void alsa_error_handler (const char* file, int line, const char* function, int err, const char* fmt, ...) {
        const auto size = 1024;
        char buffer [size];
        va_list arg;
        va_start (arg, fmt);
        snprintf(buffer, size, fmt, arg);
        va_end (arg);
    }
#endif

#ifdef __linux__
    void jack_error_handler (const char* text) {}
#endif

int main(int argc, char* argv[]) {
    #ifdef __linux__
        snd_lib_error_set_handler(alsa_error_handler);
        jack_set_error_function(jack_error_handler);
    #endif

    portaudio::AutoSystem autoSystem;

    for (const auto& name: device_names()) {
        std::cout << name << std::endl;
    }

    return 0;
}
