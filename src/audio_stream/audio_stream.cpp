/**
 * @file audio_stream.cpp
 * @brief Implementation of the AudioSpeaker and AudioRecorder classes for audio
 * streaming.
 */

#include "audio_stream.hpp"

#include <boost/log/trivial.hpp>
#include <iostream>

using std::array;
using std::cerr;
using std::cout;
using std::endl;
using std::runtime_error;
using std::shared_ptr;
using std::stringstream;

PaStream* AudioSpeaker::stream_{nullptr};

/**
 * @brief Constructs an AudioSpeaker object.
 * @param receiver The receiver object for audio streaming.
 */
AudioSpeaker::AudioSpeaker(shared_ptr<Receiver> receiver)
    : receiver_(receiver) {
    // Open the default audio stream
    PaError err =
        Pa_OpenDefaultStream(&stream_, 0, num_channels, sample_format,
                             sample_rate, frames_per_buffer, nullptr, nullptr);
    if (err != paNoError) {
        throw runtime_error(Pa_GetErrorText(err));
    }

    // Start the audio stream
    err = Pa_StartStream(stream_);
    if (err != paNoError) {
        throw runtime_error(Pa_GetErrorText(err));
    }

    // Start the receiver
    receiver_->start();

    // Clear the screen
    cout << "\033[2J\033[1;1H";

    BOOST_LOG_TRIVIAL(info) << "AudioSpeaker started";
}

/**
 * @brief Destroys the AudioSpeaker object.
 */
AudioSpeaker::~AudioSpeaker() {
    // Stop the receiver
    receiver_->stop();

    // Close the audio stream
    if (stream_) {
        PaError err = Pa_CloseStream(stream_);
        if (err != paNoError) {
            cerr << Pa_GetErrorText(err) << endl;
        }
        stream_ = nullptr;
    }

    BOOST_LOG_TRIVIAL(info) << "AudioSpeaker stopped";
}

/**
 * @brief Static callback function to handle received audio data.
 * @param buf The buffer containing the received audio data.
 * @param recv_bytes The number of received bytes.
 */
void AudioSpeaker::handle_receive_cb(uint8_t buf[], size_t recv_bytes) {
    if (!stream_) {
        return;
    }

    stringstream ss;
    sample write_buf[frames_per_buffer][num_channels] = {{sample_silence}};
    unsigned long i, j;

    // Process the received audio data
    for (i = 0; i < recv_bytes; ++i) {
        ss << buf[i];
    }

    for (i = 0; i < frames_per_buffer; ++i) {
        for (j = 0; j < num_channels; ++j) {
            ss >> write_buf[i][j];
        }
    }

    // Write the audio data to the stream
    Pa_WriteStream(stream_, write_buf, frames_per_buffer);
}

/**
 * @brief Constructs an AudioRecorder object.
 * @param sender The sender object for audio streaming.
 */
AudioRecorder::AudioRecorder(shared_ptr<Sender> sender) : sender_(sender) {
    // Open the default audio stream
    PaError err = Pa_OpenDefaultStream(&stream_, num_channels, 0, sample_format,
                                       sample_rate, frames_per_buffer,
                                       recordCallback, this);
    if (err != paNoError) {
        throw runtime_error(Pa_GetErrorText(err));
    }

    // Start the audio stream
    err = Pa_StartStream(stream_);
    if (err != paNoError) {
        throw runtime_error(Pa_GetErrorText(err));
    }

    // Clear the screen
    cout << "\033[2J\033[1;1H";

    BOOST_LOG_TRIVIAL(info) << "AudioRecorder started";
}

/**
 * @brief Destroys the AudioRecorder object.
 */
AudioRecorder::~AudioRecorder() {
    // Close the audio stream
    if (stream_) {
        PaError err = Pa_CloseStream(stream_);
        if (err != paNoError) {
            cerr << Pa_GetErrorText(err) << endl;
        }
        stream_ = nullptr;
    }

    BOOST_LOG_TRIVIAL(info) << "AudioRecorder stopped";
}

/**
 * @brief Sends a message using the sender object.
 * @param msg The message to be sent.
 */
inline void AudioRecorder::send(const std::string& msg) const {
    sender_->send(msg);
}

/**
 * @brief Callback function for recording audio.
 * @param inputBuffer The input buffer for audio data.
 * @param outputBuffer The output buffer for audio data.
 * @param framesPerBuffer The number of frames per buffer.
 * @param timeInfo The time information for the callback.
 * @param statusFlags The status flags for the callback.
 * @param userData User data passed to the callback.
 * @return The status of the callback.
 */
int AudioRecorder::recordCallback(const void* inputBuffer, void* outputBuffer,
                                  unsigned long framesPerBuffer,
                                  const PaStreamCallbackTimeInfo* timeInfo,
                                  PaStreamCallbackFlags statusFlags,
                                  void* userData) {
    AudioRecorder* recorder = (AudioRecorder*)userData;
    const sample* rptr = (const sample*)inputBuffer;
    stringstream ss;
    unsigned long i, j;

    (void)outputBuffer; /* Prevent unused variable warnings. */
    (void)timeInfo;
    (void)statusFlags;

    // Process the recorded audio data
    for (i = 0; i < framesPerBuffer; ++i) {
        for (j = 0; j < num_channels; ++j) {
            ss << *rptr++ << "\n";
        }
    }

    // Send the recorded audio data
    recorder->send(ss.str());

    return paContinue;
}
