/**
 * @file sender.hpp
 * @brief The declarations of the Sender interface and its implementations.
 */

#ifndef SENDER_HPP
#define SENDER_HPP

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>
#include <boost/regex.hpp>
#include <boost/thread/mutex.hpp>

#include "../boost_asio_bluetooth/wrapper.h"
#include "audio_config.hpp"

/**
 * @class Sender
 * @brief Abstract base class for sending data.
 */
class Sender {
  public:
    /**
     * @brief Destructor.
     */
    virtual ~Sender() {}

    /**
     * @brief Sends the provided message.
     * @param msg The message to be sent.
     */
    virtual inline void send(const std::string_view& msg) = 0;
};

/**
 * @class UDPSender
 * @brief Class for sending data over UDP.
 */
class UDPSender : public Sender {
  public:
    UDPSender(const UDPSender&) = delete;
    UDPSender(const UDPSender&&) = delete;
    UDPSender& operator=(const UDPSender&) = delete;

    /**
     * @brief Constructs a UDPSender object.
     * @param ip The IP address to send the data to.
     * @param port The port number to send the data to.
     */
    explicit UDPSender(const std::string& ip, int port)
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

        BOOST_LOG_TRIVIAL(info) << "UDPSender started";
    }

    /**
     * @brief Destroys the UDPSender object.
     */
    ~UDPSender() {
        socket_.close();

        BOOST_LOG_TRIVIAL(info) << "UDPSender stopped";
    }

    /**
     * @brief Sends the provided message over UDP.
     * @param msg The message to be sent.
     */
    virtual inline void send(const std::string_view& msg) override {
        socket_.send_to(boost::asio::buffer(msg),
                        boost::asio::ip::udp::endpoint(
                            boost::asio::ip::address::from_string(ip_), port_));
    }

  private:
    /**
     * Check if a given port number is valid.
     * @param port The port number to check.
     * @return True if the port number is valid, false otherwise.
     */
    static bool is_valid_port(int port) { return (port >= 1 && port <= 65535); }

    /**
     * Check if a given IP address is valid.
     * @param ip The IP address to check.
     * @return True if the IP address is valid, false otherwise.
     */
    static bool is_valid_ip(const std::string& ip) {
        boost::system::error_code ec;
        boost::asio::ip::address::from_string(ip, ec);
        return !ec;
    }

    const std::string ip_; /**< The IP address to send the data to. */
    const int port_;       /**< The port number to send the data to. */
    boost::asio::io_service
        io_service_; /**< The IO service for asynchronous operations. */
    boost::asio::ip::udp::socket
        socket_; /**< The UDP socket for sending data. */
};

/**
 * @class BluetoothSender
 * @brief Class for sending data over bluetooth.
 */
class BluetoothSender : public Sender, public Connection {
  public:
    BluetoothSender(const BluetoothSender&) = delete;
    BluetoothSender(const BluetoothSender&&) = delete;
    BluetoothSender& operator=(const BluetoothSender&) = delete;

    /**
     * @brief Constructs a BluetoothSender object.
     * @param ip The IP address to send the data to.
     * @param port The port number to send the data to.
     */
    explicit BluetoothSender(boost::shared_ptr<Hive> hive,
                             const std::string& mac_address, int port)
        : mac_address_(mac_address),
          port_(port),
          polling_(false),
          Connection(hive) {
        // Validate IP and Port
        if (!is_valid_port(port_)) {
            throw std::runtime_error(
                "Invalid port number. Port must be between 1 and 30.\n");
        }
        if (!is_valid_mac(mac_address_)) {
            throw std::runtime_error("Invalid MAC address.\n");
        }

        Connect(mac_address_, port_);
        polling_ = true;
        thread_ = std::thread([&] {
            while (polling_) {
                GetHive()->Poll();
            }
        });

        BOOST_LOG_TRIVIAL(info) << "BluetoothSender started";
    }

    /**
     * @brief Destroys the BluetoothSender object.
     */
    ~BluetoothSender() {
        polling_ = false;
        if (thread_.joinable()) {
            thread_.join();
        }
        GetHive()->Stop();

        BOOST_LOG_TRIVIAL(info) << "BluetoothSender stopped";
    }

    /**
     * @brief Sends the provided message over UDP.
     * @param msg The message to be sent.
     */
    virtual inline void send(const std::string_view& msg) override {
        std::vector<uint8_t> request;
        std::copy(msg.begin(), msg.end(), std::back_inserter(request));
        Send(request);
    }

  private:
    /**
     * Check if a given port number is valid.
     * @param port The port number to check.
     * @return True if the port number is valid, false otherwise.
     */
    static bool is_valid_port(int port) { return (port >= 1 && port <= 30); }

    /**
     * Check if a given MAC address is valid.
     * @param mac_address The MAC address to check.
     * @return True if the MAC address is valid, false otherwise.
     */
    static bool is_valid_mac(const std::string& mac_address) {
        static const boost::regex mac_regex(
            "^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$",
            boost::regex_constants::icase);
        return boost::regex_match(mac_address, mac_regex);
    }

    /**
     * Handles the accept event for a given MAC address and channel.
     *
     * @param mac_addr the MAC address of the accepted connection
     * @param channel the channel of the accepted connection
     */
    void OnAccept(const std::string& mac_addr, uint8_t channel) override {
        stream_lock_.lock();
        BOOST_LOG_TRIVIAL(info)
            << "[OnAccept] " << mac_addr << ":" << channel << "\n";
        stream_lock_.unlock();

        Recv();
    }

    /**
     * OnConnect function is called when a connection is established.
     *
     * @param mac_addr the MAC address of the connected device
     * @param channel the channel used for the connection
     */
    void OnConnect(const std::string& mac_addr, uint8_t channel) override {
        BOOST_LOG_TRIVIAL(info)
            << "[OnConnect] " << mac_addr << ":" << channel << "\n";
        stream_lock_.unlock();

        Recv();
    }

    /**
     * Sends a std::vector buffer.
     *
     * @param buffer The std::vector buffer containing the data to be sent.
     */
    void OnSend(const std::vector<uint8_t>& buffer) override {}

    /**
     * Receives data from a std::vector buffer and processes it.
     *
     * @param buffer The std::vector buffer containing the data to be processed.
     */
    void OnRecv(std::vector<uint8_t>& buffer) override {
        // Start the next receive
        Recv();
    }

    /**
     * OnTimer is called when timer expires.
     *
     * @param delta The time elapsed since the last call to OnTimer.
     */
    void OnTimer(const boost::posix_time::time_duration& delta) override {}

    /**
     * Handles an error that occurs during the execution of the function.
     *
     * @param error The error code that occurred.
     */
    void OnError(const boost::system::error_code& error) override {
        stream_lock_.lock();
        BOOST_LOG_TRIVIAL(error) << "[OnError] " << error << "\n";
        stream_lock_.unlock();
    }

  private:
    const std::string mac_address_; /**< The MAC address to send the data to. */
    const int port_;                /**< The port number to listen on. */
    boost::mutex stream_lock_;      /**< The stream lock. */
    std::atomic<bool> polling_; /**< Whether the polling thread is running. */
    std::thread thread_;        /**< Polling thread for receiving data. */
};

#endif  // SENDER_HPP
