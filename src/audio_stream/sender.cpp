/**
 * @file sender.cpp
 * @brief The definitions of the Sender interface and its implementations.
 */

#include "sender.hpp"

#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>
#include <boost/make_shared.hpp>
#include <boost/regex.hpp>

#include "audio_config.hpp"

/**
 * @brief Constructs a UDPSender object.
 * @param ip The IP address to send the data to.
 * @param port The port number to send the data to.
 */
UDPSender::UDPSender(const std::string& ip, int port)
    : ip_(ip),
      port_(port),
      io_service_(boost::asio::io_service()),
      socket_(boost::asio::ip::udp::socket(io_service_)) {
    // Validate IP and Port
    if (!is_valid_port(port_)) {
        throw std::runtime_error(
            "Invalid port number. Port must be between 1 and 65535.\n");
    }
    if (!is_valid_ip(ip_)) {
        throw std::runtime_error("Invalid IP address.\n");
    }

    socket_.open(boost::asio::ip::udp::v4());

    BOOST_LOG_TRIVIAL(info) << "UDPSender created";
}

/**
 * @brief Destroys the UDPSender object.
 */
UDPSender::~UDPSender() {
    socket_.close();

    BOOST_LOG_TRIVIAL(info) << "UDPSender destroyed";
}

/**
 * @brief Sends the provided message over UDP.
 * @param msg The message to be sent.
 */
void UDPSender::send(const std::string& msg) {
    socket_.send_to(boost::asio::buffer(msg),
                    boost::asio::ip::udp::endpoint(
                        boost::asio::ip::address::from_string(ip_), port_));
}

bool UDPSender::is_valid_port(int port) { return (port >= 1 && port <= 65535); }

/**
 * Check if a given IP address is valid.
 * @param ip The IP address to check.
 * @return True if the IP address is valid, false otherwise.
 */
bool UDPSender::is_valid_ip(const std::string& ip) {
    boost::system::error_code ec;
    boost::asio::ip::address::from_string(ip, ec);
    return !ec;
}

/**
 * @brief Constructs a BluetoothSender object.
 * @param mac_address The MAC address to send the data to.
 * @param port The port number to send the data to.
 */
BluetoothSender::BluetoothSender(const std::string& mac_address)
    : mac_address_(mac_address) {
    if (!is_valid_mac(mac_address_)) {
        throw std::runtime_error("Invalid MAC address.\n");
    }

    try {
        device_inq_ = std::unique_ptr<DeviceINQ>(DeviceINQ::Create());
        channel_ =
            device_inq_->SdpSearch(mac_address, "GENERIC_AUDIO_PROFILE_ID");
    } catch (const BluetoothException& e) {
        throw std::runtime_error("Cannot get channel for " + mac_address_ +
                                 "\n\t" + e.what() + "\n");
    }
    BOOST_LOG_TRIVIAL(info)
        << "Channel " << channel_ << " found for " << mac_address_;

    try {
        binding_ = std::unique_ptr<BTSerialPortBinding>(
            BTSerialPortBinding::Create(mac_address, channel_));
        binding_->Connect();
    } catch (const BluetoothException& e) {
        throw std::runtime_error("Cannot connect to " + mac_address_ +
                                 " on channel " + std::to_string(channel_) +
                                 "\n\t" + e.what() + "\n");
    }

    BOOST_LOG_TRIVIAL(info)
        << "BluetoothSender is connected to " << mac_address_;
}

/**
 * @brief Destroys the BluetoothSender object.
 */
BluetoothSender::~BluetoothSender() {
    binding_->Close();

    BOOST_LOG_TRIVIAL(info) << "BluetoothSender destroyed";
}

/**
 * @brief Sends the provided message over UDP.
 * @param msg The message to be sent.
 */
void BluetoothSender::send(const std::string& msg) {
    binding_->Write(msg.c_str(), msg.size());
}

/**
 * Check if a given MAC address is valid.
 * @param mac_address The MAC address to check.
 * @return True if the MAC address is valid, false otherwise.
 */
bool BluetoothSender::is_valid_mac(const std::string& mac_address) {
    static const boost::regex mac_regex(
        "^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$",
        boost::regex_constants::icase);
    return boost::regex_match(mac_address, mac_regex);
}
