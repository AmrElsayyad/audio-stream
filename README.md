# Audio Stream

[![Test](https://github.com/AmrElsayyad/audio-stream/actions/workflows/test.yml/badge.svg)](https://github.com/AmrElsayyad/audio-stream/actions/workflows/test.yml)

This project is designed for audio streaming functionality. It provides the ability to record, send, receive, and play audio.

## Getting Started

To get started with this project:

- Clone the repository to your local machine.
- You need to have `CMake >= 3.21` installed, and `MinGW` on Windows.
- Build the project using `./build.sh` on Unix-based systems, or `.\build.bat` on Windows.
- Optionally, you can run unit tests using `./build/test/UnitTests`.
- Run `./build/src/AudioStream` to start the audio streaming application.

## Code Overview

src/audio_streamer.hpp: This file contains the definitions of the AudioPlayer and AudioRecorder classes. The AudioPlayer class is responsible for receiving and playing audio, and the AudioRecorder class is used for recording and sending audio.

src/main.cpp: This file contains the main function. It initializes the AudioPlayer and AudioRecorder and runs the main event loop.

## Usage

To use the audio streaming application, follow these steps:

- You can show the help message by running `./AudioStream -h`.
- If you want to start as a player listening for recorders, use the `-p` or `--player` option followed by the port number.
  For example: `./AudioStream -p 1234`
- If you want to start as a recorder sending to a player, use the `-r` or `--recorder` option followed by the IP address and port number.
  For example: `./AudioStream -r 192.168.0.1:1234`
- Enter 'q' to quit the application.

## Contributing

Contributions are welcome. Please feel free to submit a pull request or open an issue if you encounter any problems or have suggestions for improvements.

## Support

For any questions or support, please email: amrelsayyad96@outlook.com

Enjoy using Audio Stream!