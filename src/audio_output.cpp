#include "audio_output.h"

#ifdef __linux__
    void alsa_error_handler (const char* file, int line, const char* function, int err, const char* fmt, ...) {
        char buffer [1024];
        va_list arg;
        va_start (arg, fmt);
        snprintf(buffer, sizeof(buffer), fmt, arg);
        va_end (arg);
    }
#endif

#ifdef __linux__
    void jack_error_handler (const char* text) {}
#endif


void AudioOutput::initializeAudio() {
    try
    {
        #ifdef __linux__
            snd_lib_error_set_handler(alsa_error_handler);
            jack_set_error_function(jack_error_handler);
        #endif

        raiiSystem_ = std::make_unique<portaudio::AutoSystem>();
        audioSystem_ = &portaudio::System::instance();
    }
    catch (const std::exception& error)
    {
        log_->critical("cannot initialize audio output, {}", error.what());
    }
}
