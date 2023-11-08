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
    boost::shared_ptr<Hive> hive(new Hive());
    unique_ptr<AudioPlayer> player_ptr;
    unique_ptr<AudioRecorder> recorder_ptr;

    desc.add_options()  // Pairs of (name, [value_semantic,] description)
        ("help,h", "  display this help and exit")  // help
        ("speaker,s", po::value<int>(),
         "  start as speaker listening for recorders [arg = port]")  // player
        ("recorder,r", "  start as recorder sending to a player")    // recorder
        ("mac-address,m", po::value<string>(),
         "  player's MAC address (used with recorders)")  // mac_address
        ("port,p", po::value<int>(),
         "  player's port (used with recorders)");  // port

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
                "--recorder ] -m [ --mac-address ] mac_address -p [ --port ] "
                "port' not both\n";
        return 1;
    } else if (var_map.count("speaker")) {
        int port = var_map["speaker"].as<int>();

        // Start as audio player with the specified port
        player_ptr = make_unique<AudioPlayer>(make_unique<BluetoothReceiver>(
            hive, port, AudioPlayer::handle_receive_cb));

    } else if (var_map.count("recorder")) {
        if (!var_map.count("mac-address")) {
            cout << "You must specify '-m [ --mac-address ] mac_address'\n";
            return 1;
        }
        if (!var_map.count("port")) {
            cout << "You must specify '-p [ --port ] port'\n";
            return 1;
        }

        string mac_address = var_map["mac-address"].as<string>();
        int port = var_map["port"].as<int>();

        // Start as audio recorder with the specified MAC address and port
        recorder_ptr = make_unique<AudioRecorder>(
            make_unique<BluetoothSender>(hive, mac_address, port));

    } else {
        cout << "You must specify either '-s [ --speaker ] port' or '-r [ "
                "--recorder ] -m [ --mac-address ] mac_address -p [ --port ] "
                "port'\n";
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

    // Release resources
    player_ptr.release();
    recorder_ptr.release();

    // Terminate PortAudio
    Pa_Terminate();

    return 0;
}
