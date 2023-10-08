/**
 * @file audio_config.hpp
 * @brief Configuration constants and typedefs for audio processing.
 */

#ifndef AUDIO_CONFIG_HPP
#define AUDIO_CONFIG_HPP

#include <string>

#include "portaudio.h"

/// The sample rate for audio processing.
static constexpr const unsigned int sample_rate = 44100;

/// The number of frames per buffer for audio processing.
static constexpr const unsigned int frames_per_buffer = 16;

/// The number of channels for audio processing.
static constexpr const unsigned int num_channels = 2;

/// The sample format for audio processing.
static constexpr const unsigned int sample_format = paInt16;

/// The value representing silence in the sample format.
static constexpr const unsigned int sample_silence = 0;

/// The format string for printing a sample value.
static constexpr const std::string_view printf_s_format = "%d";

/// The typedef for a sample in the audio processing.
using sample = int16_t;

#endif  // AUDIO_CONFIG_HPP
