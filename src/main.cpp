/**
 * @file main.cpp
 * @brief The main entry point of the program and command-line options
 * for starting as a player or recorder.
 */

#include <boost/program_options.hpp>

#include "audio_streamer.hpp"

using std::cin;
using std::cout;
using std::endl;
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
    unique_ptr<AudioPlayer> player_ptr;
    unique_ptr<AudioRecorder> recorder_ptr;

    desc.add_options()  // Pairs of (name, [value_semantic,] description)
        ("help,h", "  display this help and exit")  // help
        ("player,p", po::value<int>(),
         "  start as player listening for recorders [arg = port]")  // player
        ("recorder,r", po::value<string>(),
         "  start as recorder sending to a player [arg = mac_address:port]");  // recorder

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

    // Check if there is an error initializing PortAudio
    if (err != paNoError) {
        throw runtime_error(Pa_GetErrorText(err));
    }

    if (var_map.count("player") && (var_map.count("recorder"))) {
        cout << "You must specify either '-p [ --player ] port' or '-r [ "
                "--recorder ] mac_address:port' not both\n";
        return 1;
    } else if (var_map.count("player")) {
        int port = var_map["player"].as<int>();

        // Start as audio player with the specified port
        player_ptr = make_unique<AudioPlayer>(make_unique<BluetoothReceiver>(
            boost::shared_ptr<Hive>(), port, AudioPlayer::handle_receive_cb));

    } else if (var_map.count("recorder")) {
        string mac_port = var_map["recorder"].as<string>();
        size_t colon_pos = mac_port.find(':');
        if (colon_pos == string::npos) {
            cout << "Invalid argument. Use format MAC_ADDRESS:PORT.\n";
            return 1;
        }

        int port = stoi(mac_port.substr(colon_pos + 1));
        string mac_address = mac_port.substr(0, colon_pos);

        // Start as audio recorder with the specified MAC address and port
        recorder_ptr = make_unique<AudioRecorder>(make_unique<BluetoothSender>(
            boost::shared_ptr<Hive>(), mac_address, port));

    } else {
        cout << "You must specify either '-p [ --player ] port' or '-r "
                "[--recorder ] mac_address:port'\n";
        return 1;
    }

    // Main loop
    while (true) {
        cout << "Enter q to quit\t";
        string input;
        getline(cin, input);
        if (input == "q" || input == "Q") {
            break;
        }
    }

    // Terminate PortAudio
    Pa_Terminate();

    return 0;
}
