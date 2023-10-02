#ifndef AUDIO_CONFIG_HPP
#define AUDIO_CONFIG_HPP

#include <string>

#include "portaudio.h"

static constexpr const unsigned int sample_rate = 44100;
static constexpr const unsigned int frames_per_buffer = 16;
static constexpr const unsigned int num_channels = 2;
static constexpr const unsigned int sample_format = paInt16;
static constexpr const unsigned int sample_silence = 0;
static constexpr const std::string_view printf_s_format = "%d";

using sample = int16_t;

#endif  // AUDIO_CONFIG_HPP
