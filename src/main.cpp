/**
 * @file main.cpp
 * @brief The main entry point of the program and command-line options
 * for starting as a player or recorder.
 */

#include <boost/program_options.hpp>
#include <iostream>

#include "audio_stream/audio_stream.hpp"

using std::cin;
using std::cout;
using std::endl;
using std::make_shared;
using std::make_unique;
using std::runtime_error;
using std::string;
using std::unique_ptr;

namespace po = boost::program_options;

/**
 * Main entry point of the program.
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return The exit status of the program.
 */
int main(int argc, char* argv[]) {
    po::options_description desc("Options");
    po::variables_map var_map;
    PaError err{paNoError};
    unique_ptr<AudioSpeaker> speaker_ptr;
    unique_ptr<AudioRecorder> recorder_ptr;

    desc.add_options()  // Pairs of (name, [value_semantic,] description)
        ("help,h", "  display this help and exit")  // help
        ("speaker,s", po::value<int>(),
         "  start as speaker listening for recorders [arg = port]")  // player
        ("recorder,r", po::value<string>(),
         "  start as recorder sending to a player [arg = mac_address]");  // recorder

    try {
        po::store(po::parse_command_line(argc, argv, desc), var_map);
    } catch (boost::wrapexcept<po::invalid_command_line_syntax> e) {
        cout << e.what() << endl << desc << endl;
        return 1;
    }
    po::notify(var_map);

    if (var_map.count("help")) {
        cout << desc << endl;
        return 1;
    }

    // Initialize PortAudio
    err = Pa_Initialize();

    // Clear the screen
    cout << "\033[2J\033[1;1H";

    // Check if there is an error initializing PortAudio
    if (err != paNoError) {
        throw runtime_error(Pa_GetErrorText(err));
    }

    if (var_map.count("speaker") && (var_map.count("recorder"))) {
        cout << "You must specify either '-s [ --speaker ] port' or '-r [ "
                "--recorder ] mac_address' not both\n";
        return 1;
    } else if (var_map.count("speaker")) {
        int port = var_map["speaker"].as<int>();

        // Start as audio player with the specified port
        speaker_ptr = make_unique<AudioSpeaker>(make_shared<BluetoothReceiver>(
            port, AudioSpeaker::handle_receive_cb));

    } else if (var_map.count("recorder")) {
        string mac_address = var_map["recorder"].as<string>();

        // Start as audio recorder with the specified MAC address and port
        recorder_ptr = make_unique<AudioRecorder>(
            make_shared<BluetoothSender>(mac_address));

    } else {
        cout << "You must specify either '-s [ --speaker ] port' or '-r [ "
                "--recorder ] mac_address'\n";
        return 1;
    }

    // Main loop
    while (true) {
        cout << "Enter q to quit\n";
        string input;
        getline(cin, input);
        if (input == "q" || input == "Q") {
            break;
        }
    }

    // Release resources
    speaker_ptr.release();
    recorder_ptr.release();

    // Terminate PortAudio
    Pa_Terminate();

    return 0;
}
