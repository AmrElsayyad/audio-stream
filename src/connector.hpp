/**
 * @file connector.hpp
 * @brief The declarations of the Sender, UDPSender, Receiver, and UDPReceiver
 * classes for network communication.
 */

#ifndef CONNECTOR_HPP
#define CONNECTOR_HPP

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>

#include "audio_config.hpp"

static constexpr const unsigned int BUFFER_SIZE = 8192;

/**
 * @class Receiver
 * @brief Abstract base class for receiving data.
 */
class Receiver {
  public:
    /**
     * @brief Constructs a Receiver object.
     * @param id The identifier for the receiver.
     * @param handle_receive_cb The callback function for handling received
     * data.
     */
    explicit Receiver(std::function<void(std::array<char, BUFFER_SIZE> buffer,
                                         size_t recv_bytes)>
                          handle_receive_cb) {
        handle_receive_cb_ = handle_receive_cb;
    }

    /**
     * @brief Starts the receiver.
     */
    virtual inline void start() = 0;

    /**
     * @brief Stops the receiver.
     */
    virtual inline void stop() = 0;

  protected:
    std::function<void(std::array<char, BUFFER_SIZE> buffer, size_t recv_bytes)>
        handle_receive_cb_; /**< The callback function for handling received
                               data. */
};

/**
 * @class UDPReceiver
 * @brief Class for receiving data over UDP.
 */
class UDPReceiver : public Receiver {
  public:
    UDPReceiver(const UDPReceiver&) = delete;
    UDPReceiver(const UDPReceiver&&) = delete;
    UDPReceiver& operator=(const UDPReceiver&) = delete;

    /**
     * @brief Constructs a UDPReceiver object.
     * @param port The port number to listen on.
     * @param handle_receive_cb Callback function for handling received data.
     */
    explicit UDPReceiver(
        int port, std::function<void(std::array<char, BUFFER_SIZE> buffer,
                                     size_t recv_bytes)>
                      handle_receive_cb)
        : Receiver(handle_receive_cb),
          port_(port),
          io_service_(boost::asio::io_service()),
          socket_(boost::asio::ip::udp::socket(io_service_)) {
        // Validate Port
        if (!is_valid_port(port)) {
            throw std::runtime_error(
                "Invalid port number. Port must be between 1 and 65535.\n");
        }
    }

    /**
     * @brief Destroys the UDPReceiver object.
     */
    ~UDPReceiver() {
        stop();
        handle_receive_cb_ = nullptr;
    }

    /**
     * @brief Starts the UDP receiver.
     */
    virtual inline void start() override {
        socket_.open(boost::asio::ip::udp::v4());
        socket_.bind(boost::asio::ip::udp::endpoint(
            boost::asio::ip::address_v4::loopback(), port_));

        thread_ = std::thread([&] {
            wait();
            io_service_.run();
        });

        BOOST_LOG_TRIVIAL(info)
            << "UDPReceiver started listening on port: " << port_;
    }

    /**
     * @brief Stops the UDP receiver.
     */
    virtual inline void stop() override {
        io_service_.stop();

        if (thread_.joinable()) {
            thread_.join();
        }

        socket_.close();

        BOOST_LOG_TRIVIAL(info)
            << "UDPReceiver stopped listening on port: " << port_;
    }

    /**
     * @brief Gets the port number that the receiver is listening on.
     * @return The port number.
     */
    inline const int& get_port() const { return port_; }

    /**
     * @brief Gets the remote endpoint from which data is received.
     * @return The remote endpoint.
     */
    inline const boost::asio::ip::udp::endpoint& get_remote_endpoint() const {
        return remote_endpoint_;
    }

  private:
    /**
     * Check if a given port number is valid.
     * @param port The port number to check.
     * @return True if the port number is valid, false otherwise.
     */
    static bool is_valid_port(int port) { return (port > 0 && port <= 65535); }

    /**
     * @brief Wrapper function for handling received data.
     * @param error The error code.
     * @param recv_bytes The number of received bytes.
     */
    inline void handle_receive_wrapper(const boost::system::error_code& error,
                                       size_t recv_bytes) {
        handle_receive_cb_(recv_buf_, recv_bytes);
        wait();
    }

    /**
     * @brief Waits for data to be received.
     */
    inline void wait() {
        socket_.async_receive_from(
            boost::asio::buffer(recv_buf_), remote_endpoint_,
            boost::bind(&UDPReceiver::handle_receive_wrapper, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    }

    const int port_; /**< The port number to listen on. */
    boost::asio::io_service
        io_service_; /**< The IO service for asynchronous operations. */
    boost::asio::ip::udp::socket
        socket_; /**< The UDP socket for receiving data. */
    std::array<char, BUFFER_SIZE>
        recv_buf_; /**< The receive buffer for incoming data. */
    boost::asio::ip::udp::endpoint remote_endpoint_; /**< The remote endpoint
                                           from which data is received. */
    std::thread thread_; /**< The thread for running the IO service. */
};

/**
 * @class Sender
 * @brief Abstract base class for sending data.
 */
class Sender {
  public:
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
        if (!is_valid_port(port)) {
            throw std::runtime_error(
                "Invalid port number. Port must be between 1 and 65535.\n");
        }
        if (!is_valid_ip(ip)) {
            throw std::runtime_error("Invalid IP address.\n");
        }
        if (!is_ip_reachable(ip)) {
            throw std::runtime_error("IP: " + ip + " is unreachable.\n");
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
    static bool is_valid_port(int port) { return (port > 0 && port <= 65535); }

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

    /**
     * Check if a given IP address is reachable.
     * @param ip The IP address to check.
     * @return True if the IP address is reachable, false otherwise.
     */
    static bool is_ip_reachable(const std::string& ip) {
        /* -c <count>         stop after <count> replies
        ** -w <deadline>      reply wait <deadline> in seconds
        */
        std::string command = "ping -c 1 -w 1 " + ip + " > /dev/null 2>&1";
        return system(command.c_str()) == 0;
    }

    const std::string ip_; /**< The IP address to send the data to. */
    const int port_;       /**< The port number to send the data to. */
    boost::asio::io_service
        io_service_; /**< The IO service for asynchronous operations. */
    boost::asio::ip::udp::socket
        socket_; /**< The UDP socket for sending data. */
};

#endif  // CONNECTOR_HPP
