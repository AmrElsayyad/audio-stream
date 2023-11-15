/**
 * @file receiver.cpp
 * @brief The definitions of the Receiver interface and its implementations.
 */

#include "receiver.hpp"

#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>

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
    int port,
    std::function<void(uint8_t buffer[], size_t recv_bytes)> handle_receive_cb)
    : Receiver(handle_receive_cb), port_(port) {
    try {
        device_inq_ = std::unique_ptr<DeviceINQ>(DeviceINQ::Create());
        binding_ =
            std::unique_ptr<BTSerialPortBinding>(BTSerialPortBinding::Create(
                device_inq_->GetLocalDevice().address, port_));
    } catch (const BluetoothException& e) {
        throw std::runtime_error("Cannot bind to local device:\n\t" +
                                 std::string(e.what()) + "\n");
    }
    try {
        binding_->Connect();
    } catch (const BluetoothException& e) {
    }
}

/**
 * @brief Destroys the BluetoothReceiver object.
 */
BluetoothReceiver::~BluetoothReceiver() {
    stop();
    binding_->Close();
    handle_receive_cb_ = nullptr;
}

/**
 * @brief Starts the BluetoothReceiver.
 */
inline void BluetoothReceiver::start() {
    polling_ = true;
    thread_ = std::thread([&] {
        while (polling_) {
            if (binding_->IsDataAvailable()) {
                uint8_t buffer[buffer_size];
                size_t recv_bytes = binding_->Read(buffer, buffer_size);
                handle_receive_cb_(buffer, recv_bytes);
            }
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

    BOOST_LOG_TRIVIAL(info)
        << "BluetoothReceiver stopped listening on port: " << port_;
}

/**
 * @brief Gets the port number that the receiver is listening on.
 * @return The port number.
 */
inline const int& BluetoothReceiver::get_port() const { return port_; }
