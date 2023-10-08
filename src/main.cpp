#include <boost/program_options.hpp>

#include "audio_streamer.hpp"

namespace po = boost::program_options;

bool is_valid_port(int port) { return (port > 0 && port <= 65535); }

bool is_valid_ip(const std::string& ip) {
    boost::system::error_code ec;
    ip::address::from_string(ip, ec);
    return !ec;
}

bool is_ip_reachable(const std::string& ip) {
    std::string command = "ping -c 1 -w 1 " + ip + " > /dev/null 2>&1";
    return system(command.c_str()) == 0;
}

int main(int argc, char* argv[]) {
    po::options_description desc("Options");
    desc.add_options()  // Pairs of (name, [value_semantic,] description)
        ("help,h", "  display this help and exit")  // help
        ("player,p", po::value<int>(),
         "  start as player listening for recorders [arg = port]")  // player
        ("recorder,r", po::value<std::string>(),
         "  start as recorder sending to a player [arg = ip:port]");  // recorder

    po::variables_map var_map;
    try {
        po::store(po::parse_command_line(argc, argv, desc), var_map);
    } catch (boost::wrapexcept<po::invalid_command_line_syntax> e) {
        std::cout << e.what() << std::endl << desc << std::endl;
        return 1;
    }
    po::notify(var_map);

    if (var_map.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }

    // Supress errors from "Pa_Initialize()"
    // by replacing standard error with "/dev/null"
    int saved_stderr = dup(STDERR_FILENO);
    dup2(open("/dev/null", O_RDWR), STDERR_FILENO);

    // Initialize PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        throw std::runtime_error(Pa_GetErrorText(err));
    }

    // Restore standard error
    dup2(saved_stderr, STDERR_FILENO);

    std::unique_ptr<AudioPlayer> player_ptr;
    std::unique_ptr<AudioRecorder> recorder_ptr;

    if (var_map.count("player") && (var_map.count("recorder"))) {
        std::cout << "You must specify either '-p [ --player ] port' or '-r [ "
                     "--recorder ] ip:port' not both\n";
        return 1;
    } else if (var_map.count("player")) {
        int port = var_map["player"].as<int>();
        if (!is_valid_port(port)) {
            std::cout
                << "Invalid port number. Port must be between 1 and 65535.\n";
            return 1;
        }

        // Start as audio player with the specified port
        player_ptr =
            std::make_unique<AudioPlayer>(std::make_unique<UDPReceiver>(
                port, AudioPlayer::handle_receive_cb));

    } else if (var_map.count("recorder")) {
        std::string ip_port = var_map["recorder"].as<std::string>();
        size_t colon_pos = ip_port.find(':');
        if (colon_pos == std::string::npos) {
            std::cout << "Invalid argument. Use format IP:PORT.\n";
            return 1;
        }

        int port = std::stoi(ip_port.substr(colon_pos + 1));
        if (!is_valid_port(port)) {
            std::cout
                << "Invalid port number. Port must be between 1 and 65535.\n";
            return 1;
        }

        std::string ip = ip_port.substr(0, colon_pos);
        if (!is_valid_ip(ip)) {
            std::cout << "Invalid IP address.\n";
            return 1;
        }
        if (!is_ip_reachable(ip)) {
            std::cout << "IP: " << ip << " is unreachable.\n";
            return 1;
        }

        // Start as audio recorder with the specified IP and port
        recorder_ptr = std::make_unique<AudioRecorder>(
            std::make_unique<UDPSender>(ip, port));

    } else {
        std::cout << "You must specify either '-p [ --player ] port' or '-r [ "
                     "--recorder ] ip:port'\n";
        return 1;
    }

    // Main loop
    while (true) {
        std::cout << "Enter q to quit\t";
        std::string input;
        getline(std::cin, input);
        if (input == "q" || input == "Q") {
            break;
        }
    }

    // Terminate PortAudio
    Pa_Terminate();

    return 0;
}
