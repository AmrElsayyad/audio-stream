# Audio Stream

[![Test](https://github.com/AmrElsayyad/audio-stream/actions/workflows/test.yml/badge.svg)](https://github.com/AmrElsayyad/audio-stream/actions/workflows/test.yml)

This project is designed for audio streaming functionality. It provides the ability to record, send, receive, and play audio.

## Build From Source

### Requirements

#### General Requirements

- [CMake](https://cmake.org/) >= 3.21

#### Windows

- [MinGW](https://www.mingw-w64.org/)

### Build

Run `./build.sh` on Unix-based systems, or `.\build.bat` on Windows.

### Test

Run `./build/test/UnitTests` to run the unit tests, or use `ctest` in the build directory.

### Usage

Run `./build/src/AudioStream` to start the audio streaming application.

## Code Overview

src/audio_streamer.hpp: This file contains the definitions of the AudioPlayer and AudioRecorder classes. The AudioPlayer class is responsible for receiving and playing audio, and the AudioRecorder class is used for recording and sending audio.

src/main.cpp: This file contains the main function. It initializes the AudioPlayer and AudioRecorder and runs the main event loop.

## Usage

To use the audio streaming application, follow these steps:

- You can show the help message by running `./AudioStream -h`.
- If you want to start as a player listening for recorders, use the `-p [ --player ]` option followed by the port number.
  For example: `./AudioStream -p 13`
- If you want to start as a recorder sending to a player, use the `-r [ --recorder ]` option followed by `-m [ --mac-address ] mac_address -p [ --port ] port`.
  For example: `./AudioStream -r -m 94:39:E5:8E:5A:A2 -p 13`
- Enter 'q' to quit the application.

## Contributing

Contributions are welcome. Please feel free to submit a pull request or open an issue if you encounter any problems or have suggestions for improvements.

## Support

For any questions or support, please email: amrelsayyad96@outlook.com

Enjoy using Audio Stream!