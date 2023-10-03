#ifndef AUDIO_STREAMER_HPP
#define AUDIO_STREAMER_HPP

#include <iostream>

#include "connector.hpp"

class AudioPlayer {
  public:
    AudioPlayer(const AudioPlayer &) = delete;
    AudioPlayer(const AudioPlayer &&) = delete;
    void operator=(const AudioPlayer &) = delete;

    AudioPlayer(std::unique_ptr<Receiver>(receiver));
    ~AudioPlayer();

    static void handle_receive_cb(boost::array<char, BUFFER_SIZE> buf,
                                  size_t recv_bytes);

  private:
    std::unique_ptr<Receiver> receiver_;
    static PaStream *stream_;
};

class AudioRecorder {
  public:
    AudioRecorder(const AudioRecorder &) = delete;
    AudioRecorder(const AudioRecorder &&) = delete;
    void operator=(const AudioRecorder &) = delete;

    AudioRecorder(std::unique_ptr<Sender> sender);
    ~AudioRecorder();

    bool is_playing() const;

  private:
    inline void send(const std::string_view &msg) const;

    /* This routine will be called by the PortAudio engine when audio is needed.
    ** It may be called at interrupt level on some machines so don't do anything
    ** that could mess up the system like calling malloc() or free().
    */
    static int recordCallback(const void *inputBuffer, void *outputBuffer,
                              unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo *timeInfo,
                              PaStreamCallbackFlags statusFlags,
                              void *userData);

    const std::unique_ptr<Sender> sender_;
    PaStream *stream_;
    bool playing_;
};

#endif  // AUDIO_STREAMER_HPP
