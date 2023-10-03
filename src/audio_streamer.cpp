#include "audio_streamer.hpp"

PaStream *AudioPlayer::stream_{nullptr};

AudioPlayer::AudioPlayer(std::unique_ptr<Receiver>(receiver))
    : receiver_(std::move(receiver)) {
    PaError err = paNoError;

    err =
        Pa_OpenDefaultStream(&stream_, 0, num_channels, sample_format,
                             sample_rate, frames_per_buffer, nullptr, nullptr);
    if (err != paNoError) {
        throw std::runtime_error(Pa_GetErrorText(err));
    }

    err = Pa_StartStream(stream_);
    if (err != paNoError) {
        throw std::runtime_error(Pa_GetErrorText(err));
    }

    receiver_->start();

    BOOST_LOG_TRIVIAL(info) << "AudioPlayer started";
}

AudioPlayer::~AudioPlayer() {
    receiver_->stop();

    if (stream_) {
        PaError err = Pa_CloseStream(stream_);
        if (err != paNoError) {
            std::cerr << Pa_GetErrorText(err) << std::endl;
        }
        stream_ = nullptr;
    }

    BOOST_LOG_TRIVIAL(info) << "AudioPlayer stopped";
}

void AudioPlayer::handle_receive_cb(boost::array<char, BUFFER_SIZE> buf,
                                    size_t recv_bytes) {
    if (!stream_) {
        return;
    }

    std::stringstream ss;
    sample write_buf[frames_per_buffer][num_channels] = {{sample_silence}};
    unsigned long i, j;

    for (const auto &c : buf) {
        ss << c;
    }

    for (i = 0; i < frames_per_buffer; ++i) {
        for (j = 0; j < num_channels; ++j) {
            ss >> write_buf[i][j];
        }
    }

    Pa_WriteStream(stream_, write_buf, frames_per_buffer);
}

AudioRecorder::AudioRecorder(std::unique_ptr<Sender> sender)
    : sender_(std::move(sender)), playing_(true) {
    PaError err = paNoError;

    err = Pa_OpenDefaultStream(&stream_, num_channels, 0, sample_format,
                               sample_rate, frames_per_buffer, recordCallback,
                               this);
    if (err != paNoError) {
        throw std::runtime_error(Pa_GetErrorText(err));
    }

    err = Pa_StartStream(stream_);
    if (err != paNoError) {
        throw std::runtime_error(Pa_GetErrorText(err));
    }

    BOOST_LOG_TRIVIAL(info) << "AudioRecorder started";
}

AudioRecorder::~AudioRecorder() {
    playing_ = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (stream_) {
        PaError err = Pa_CloseStream(stream_);
        if (err != paNoError) {
            std::cerr << Pa_GetErrorText(err) << std::endl;
        }
        stream_ = nullptr;
    }

    BOOST_LOG_TRIVIAL(info) << "AudioRecorder stopped";
}

bool AudioRecorder::is_playing() const { return playing_; }

inline void AudioRecorder::send(const std::string_view &msg) const {
    sender_->send(msg);
}

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
int AudioRecorder::recordCallback(const void *inputBuffer, void *outputBuffer,
                                  unsigned long framesPerBuffer,
                                  const PaStreamCallbackTimeInfo *timeInfo,
                                  PaStreamCallbackFlags statusFlags,
                                  void *userData) {
    AudioRecorder *recorder = (AudioRecorder *)userData;
    const sample *rptr = (const sample *)inputBuffer;
    std::stringstream ss;
    unsigned long i, j;

    (void)outputBuffer; /* Prevent unused variable warnings. */
    (void)timeInfo;
    (void)statusFlags;

    for (i = 0; i < framesPerBuffer; ++i) {
        for (j = 0; j < num_channels; ++j) {
            ss << *rptr++ << "\n";
        }
    }

    recorder->send(ss.str());

    if (!recorder->is_playing()) {
        return paComplete;
    }

    return paContinue;
}
