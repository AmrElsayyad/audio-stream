#include <iostream>

#include "audio_streamer.hpp"

int main(int argc, char *argv[]) {
    int port{};

    // The program expects a port number as input argument
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }
    try {
        port = std::stoi(argv[1]);
    } catch (...) {
        std::cerr << "Invalid port\n";
        return 1;
    }

    // Supress ALSA errors by replacing standard error with /dev/null
    // int saved_stderr = dup(STDERR_FILENO);
    // dup2(open("/dev/null", O_RDWR), STDERR_FILENO);

    PaError err = paNoError;

    err = Pa_Initialize();
    if (err != paNoError) {
        throw std::runtime_error(Pa_GetErrorText(err));
    }

    // Restore standard error
    // dup2(saved_stderr, STDERR_FILENO);

    // Start the player
    AudioPlayer player(std::make_unique<UDPReceiver>(port));

    while (true) {
        std::cout << "Press any key to connect or q to quit: ";
        std::string input;
        getline(std::cin, input);
        if (input == "q" || input == "Q") {
            break;
        }

        std::cout << "Enter the audio player's\nIP: ";
        std::string player_ip;
        getline(std::cin, player_ip);

        std::cout << "Port: ";
        std::string player_port_str;
        int player_port{};
        getline(std::cin, player_port_str);
        try {
            player_port = std::stoi(player_port_str);
        } catch (...) {
            std::cerr << "Invalid port\n";
            continue;
        }

        AudioRecorder recorder(
            std::make_unique<UDPSender>(player_ip, player_port));

        std::cout << "\nPress any key to stop.\n";
        getchar();
    }

    Pa_Terminate();

    return 0;
}
