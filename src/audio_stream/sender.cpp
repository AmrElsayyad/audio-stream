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
void UDPSender::send(const std::string_view& msg) {
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
 * @param ip The IP address to send the data to.
 * @param port The port number to send the data to.
 */
BluetoothSender::BluetoothSender(boost::shared_ptr<Hive> hive,
                                          const std::string& mac_address,
                                          int port)
    : connection_(boost::make_shared<BluetoothConnection>(hive)),
      mac_address_(mac_address),
      port_(port),
      polling_(false) {
    // Validate IP and Port
    if (!is_valid_port(port_)) {
        throw std::runtime_error(
            "Invalid port number. Port must be between 1 and 30.\n");
    }
    if (!is_valid_mac(mac_address_)) {
        throw std::runtime_error("Invalid MAC address.\n");
    }

    // Connect to the device
    connection_->Connect(mac_address_, port_);

    // Start the polling thread
    polling_ = true;
    thread_ = std::thread([&] {
        while (polling_) {
            connection_->GetHive()->Poll();
        }
    });

    BOOST_LOG_TRIVIAL(info) << "BluetoothSender is connected to "
                            << mac_address_ << " on port " << port_;
}

/**
 * @brief Destroys the BluetoothSender object.
 */
BluetoothSender::~BluetoothSender() {
    polling_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
    connection_->GetHive()->Stop();

    BOOST_LOG_TRIVIAL(info) << "BluetoothSender destroyed";
}

/**
 * @brief Sends the provided message over UDP.
 * @param msg The message to be sent.
 */
void BluetoothSender::send(const std::string_view& msg) {
    std::vector<uint8_t> request;
    std::copy(msg.begin(), msg.end(), std::back_inserter(request));
    connection_->Send(request);
}

/**
 * Check if a given port number is valid.
 * @param port The port number to check.
 * @return True if the port number is valid, false otherwise.
 */
bool BluetoothSender::is_valid_port(int port) {
    return (port >= 1 && port <= 30);
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

/**
 * Handles the accept event for a given MAC address and channel.
 *
 * @param mac_addr The MAC address of the accepted connection.
 * @param channel The channel of the accepted connection.
 */
void BluetoothSender::BluetoothConnection::OnAccept(const std::string& mac_addr,
                                                    uint8_t channel) {
    stream_lock_.lock();
    BOOST_LOG_TRIVIAL(info)
        << "[OnAccept] " << mac_addr << ":" << channel << "\n";
    stream_lock_.unlock();

    Recv();
}

/**
 * Handles the connect event for a given MAC address and channel.
 *
 * @param mac_addr The MAC address of the connected device.
 * @param channel The channel used for the connection.
 */
void BluetoothSender::BluetoothConnection::OnConnect(
    const std::string& mac_addr, uint8_t channel) {
    BOOST_LOG_TRIVIAL(info)
        << "[OnConnect] " << mac_addr << ":" << channel << "\n";
    stream_lock_.unlock();

    Recv();
}

/**
 * Receives data from a std::vector buffer and processes it.
 *
 * @param buffer The std::vector buffer containing the data to be
 * processed.
 */
void BluetoothSender::BluetoothConnection::OnRecv(
    std::vector<uint8_t>& buffer) {
    // Start the next receive
    Recv();
}

/**
 * Handles an error that occurs during the execution of the function.
 *
 * @param error The error code that occurred.
 */
void BluetoothSender::BluetoothConnection::OnError(
    const boost::system::error_code& error) {
    stream_lock_.lock();
    BOOST_LOG_TRIVIAL(info) << "[OnError] " << error;
    stream_lock_.unlock();
}
