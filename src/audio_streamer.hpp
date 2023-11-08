/**
 * @file audio_streamer.hpp
 * @brief The declarations of the AudioPlayer and AudioRecorder classes for
 * audio streaming.
 *
 * The AudioPlayer class provides functionality for playing audio, while the
 * AudioRecorder class allows for recording audio. Both classes utilize the
 * Receiver and Sender objects for audio streaming.
 */

#ifndef AUDIO_STREAMER_HPP
#define AUDIO_STREAMER_HPP

#include <iostream>

#include "receiver.hpp"
#include "sender.hpp"

/**
 * @defgroup sequence_diagrams Sequence Diagrams
 * @brief This topic contains the sequence diagrams for the project.
 */

/**
 * @ingroup sequence_diagrams
 *
 * @defgroup audio_player_sequence AudioPlayer Interactions
 * @brief This sequence diagram illustrates the interaction between the Sender,
 * Receiver, and AudioPlayer components for audio streaming.
 *
 * @msc
 * Sender, Receiver, AudioPlayer;
 *
 * Receiver->Receiver [label="register handle_receive_cb"];
 * AudioPlayer->AudioPlayer [label="open audio stream"];
 * AudioPlayer->Receiver [label="start"];
 * Sender->Receiver [label="receive audio data"];
 * Receiver->AudioPlayer [label="call handle_receive_cb"];
 * AudioPlayer->AudioPlayer [label="process and write data to stream"];
 * @endmsc
 *
 * \n The Receiver registers the handle_receive_cb function of the AudioPlayer.
 * The AudioPlayer opens an audio stream and starts the Receiver. The Receiver
 * then begins receiving audio data from the Sender. Upon receiving the data,
 * the Receiver calls handle_receive_cb, enabling the AudioPlayer to process the
 * received audio data and play it through the speakers by writing it to the
 * audio stream.
 */

/**
 * @class AudioPlayer
 * @brief Class for playing audio.
 */
class AudioPlayer {
  public:
    AudioPlayer(const AudioPlayer &) = delete;
    AudioPlayer(const AudioPlayer &&) = delete;
    AudioPlayer &operator=(const AudioPlayer &) = delete;

    /**
     * @brief Constructs an AudioPlayer object.
     * @param receiver A unique_ptr to the receiver object for audio streaming.
     */
    explicit AudioPlayer(std::unique_ptr<Receiver>(receiver));

    /**
     * @brief Destroys the AudioPlayer object.
     */
    ~AudioPlayer();

    /**
     * @brief Static callback function to handle received audio data.
     * @param buf The buffer containing the received audio data.
     * @param recv_bytes The number of received bytes.
     */
    static void handle_receive_cb(uint8_t buf[], size_t recv_bytes);

  private:
    const std::unique_ptr<Receiver>
        receiver_;            /**< The receiver object for audio streaming. */
    static PaStream *stream_; /**< The audio stream. */
};

/**
 * @ingroup sequence_diagrams
 *
 * @defgroup audio_recorder_sequence AudioRecorder Interactions
 * @brief This sequence diagram illustrates the interaction between the
 * AudioRecorder, Sender, and Receiver components for audio streaming.
 *
 * @msc
 * AudioRecorder, Sender, Receiver;
 *
 * AudioRecorder->AudioRecorder [label="open audio stream"];
 * AudioRecorder->AudioRecorder [label="record and process audio data"];
 * AudioRecorder->Sender [label="call send"];
 * Sender->Receiver [label="send data to receiver"];
 * @endmsc
 *
 * \n The AudioRecorder opens an audio stream and starts recording audio data.
 * The recorded audio data is processed, and then it is sent to the Receiver
 * using the Sender component.
 */

/**
 * @class AudioRecorder
 * @brief Class for recording audio.
 */
class AudioRecorder {
  public:
    AudioRecorder(const AudioRecorder &) = delete;
    AudioRecorder(const AudioRecorder &&) = delete;
    AudioRecorder &operator=(const AudioRecorder &) = delete;

    /**
     * @brief Constructs an AudioRecorder object.
     * @param sender A unique_ptr to the sender object for audio streaming.
     */
    explicit AudioRecorder(std::unique_ptr<Sender> sender);

    /**
     * @brief Destroys the AudioRecorder object.
     */
    ~AudioRecorder();

  private:
    /**
     * @brief Sends a message using the sender object.
     * @param msg The message to be sent.
     */
    inline void send(const std::string_view &msg) const;

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
    static int recordCallback(const void *inputBuffer, void *outputBuffer,
                              unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo *timeInfo,
                              PaStreamCallbackFlags statusFlags,
                              void *userData);

    const std::unique_ptr<Sender>
        sender_;       /**< The sender object for audio streaming. */
    PaStream *stream_; /**< The audio stream. */
};

#endif  // AUDIO_STREAMER_HPP
