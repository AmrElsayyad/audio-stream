/**
 * @file receiver.cpp
 * @brief The definitions of the Receiver interface and its implementations.
 */

#include "receiver.hpp"

#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>

#include "audio_config.hpp"

/**
 * @brief Constructs a UDPReceiver object.
 * @param port The port number to listen on.
 * @param handle_receive_cb Callback function for handling received data.
 */
UDPReceiver::UDPReceiver(
    int port,
    std::function<void(uint8_t buffer[], size_t recv_bytes)> handle_receive_cb)
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
UDPReceiver::~UDPReceiver() {
    stop();
    handle_receive_cb_ = nullptr;
}

/**
 * @brief Starts the UDP receiver.
 */
inline void UDPReceiver::start() {
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
inline void UDPReceiver::stop() {
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
inline const int& UDPReceiver::get_port() const { return port_; }

/**
 * @brief Gets the remote endpoint from which data is received.
 * @return The remote endpoint.
 */
inline const boost::asio::ip::udp::endpoint& UDPReceiver::get_remote_endpoint()
    const {
    return remote_endpoint_;
}

/**
 * Check if a given port number is valid.
 * @param port The port number to check.
 * @return True if the port number is valid, false otherwise.
 */
bool UDPReceiver::is_valid_port(int port) {
    return (port > 0 && port <= 65535);
}

/**
 * @brief Wrapper function for handling received data.
 * @param error The error code.
 * @param recv_bytes The number of received bytes.
 */
inline void UDPReceiver::handle_receive_wrapper(
    const boost::system::error_code& error, size_t recv_bytes) {
    handle_receive_cb_(recv_buf_.data(), recv_bytes);
    wait();
}

/**
 * @brief Waits for data to be received.
 */
inline void UDPReceiver::wait() {
    socket_.async_receive_from(
        boost::asio::buffer(recv_buf_), remote_endpoint_,
        boost::bind(&UDPReceiver::handle_receive_wrapper, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

/**
 * @brief Constructs a BluetoothReceiver object.
 * @param port The port number to listen on.
 * @param handle_receive_cb Callback function for handling received data.
 */
BluetoothReceiver::BluetoothReceiver(
    boost::shared_ptr<Hive> hive, int port,
    std::function<void(uint8_t buffer[], size_t recv_bytes)> handle_receive_cb)
    : Receiver(handle_receive_cb),
      port_(port),
      polling_(false),
      acceptor_(new ConnectionAcceptor(hive)),
      Connection(hive) {}

/**
 * @brief Destroys the BluetoothReceiver object.
 */
BluetoothReceiver::~BluetoothReceiver() {
    stop();
    handle_receive_cb_ = nullptr;
}

/**
 * @brief Starts the BluetoothReceiver.
 */
inline void BluetoothReceiver::start() {
    acceptor_->Listen(port_);
    acceptor_->Accept(boost::shared_ptr<BluetoothReceiver>(this));
    polling_ = true;
    thread_ = std::thread([&] {
        while (polling_) {
            GetHive()->Poll();
        }
    });

    BOOST_LOG_TRIVIAL(info)
        << "BluetoothReceiver started listening on port: " << port_;
}

/**
 * @brief Stops the BluetoothReceiver.
 */
inline void BluetoothReceiver::stop() {
    polling_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
    GetHive()->Stop();

    BOOST_LOG_TRIVIAL(info)
        << "BluetoothReceiver stopped listening on port: " << port_;
}

/**
 * @brief Gets the port number that the receiver is listening on.
 * @return The port number.
 */
inline const int& BluetoothReceiver::get_port() const { return port_; }

/**
 * Handles the accept event for a given MAC address and channel.
 *
 * @param mac_addr the MAC address of the accepted connection
 * @param channel the channel of the accepted connection
 */
void BluetoothReceiver::OnAccept(const std::string& mac_addr, uint8_t channel) {
    acceptor_->stream_lock_.lock();
    BOOST_LOG_TRIVIAL(info)
        << "[OnAccept] " << mac_addr << ":" << channel << "\n";
    acceptor_->stream_lock_.unlock();

    Recv();
}

/**
 * OnConnect function is called when a connection is established.
 *
 * @param mac_addr the MAC address of the connected device
 * @param channel the channel used for the connection
 */
void BluetoothReceiver::OnConnect(const std::string& mac_addr,
                                  uint8_t channel) {
    BOOST_LOG_TRIVIAL(info)
        << "[OnConnect] " << mac_addr << ":" << channel << "\n";
    acceptor_->stream_lock_.unlock();

    Recv();
}

/**
 * Receives data from a std::vector buffer and processes it.
 *
 * @param buffer The std::vector buffer containing the data to be processed.
 */
void BluetoothReceiver::OnRecv(std::vector<uint8_t>& buffer) {
    acceptor_->stream_lock_.lock();
    handle_receive_cb_(buffer.data(), buffer.size());
    acceptor_->stream_lock_.unlock();

    Recv();
}

/**
 * Handles an error that occurs during the execution of the function.
 *
 * @param error The error code that occurred.
 */
void BluetoothReceiver::OnError(const boost::system::error_code& error) {
    acceptor_->stream_lock_.lock();
    BOOST_LOG_TRIVIAL(info) << "[OnError] " << error;
    acceptor_->stream_lock_.unlock();
}

/**
 * @brief Handles the accept event for a connection.
 *
 * @param connection A shared pointer to the Connection object.
 * @param addr The address of the connection.
 * @param channel The channel of the connection.
 *
 * @return true if the accept event was handled successfully.
 */
bool BluetoothReceiver::ConnectionAcceptor::OnAccept(
    boost::shared_ptr<Connection> connection, const std::string& addr,
    uint8_t channel) {
    stream_lock_.lock();
    BOOST_LOG_TRIVIAL(info) << "[OnAccept] " << addr << ":" << channel;
    stream_lock_.unlock();

    return true;
}

/**
 * @brief Handles an error that occurs in the program.
 *
 * @param error The error code representing the error that occurred.
 */
void BluetoothReceiver::ConnectionAcceptor::OnError(
    const boost::system::error_code& error) {
    stream_lock_.lock();
    BOOST_LOG_TRIVIAL(info) << "[OnError] " << error;
    stream_lock_.unlock();
}
