/**
 * @file receiver.hpp
 * @brief The declarations of the Receiver interface and its implementations.
 */

#ifndef RECEIVER_HPP
#define RECEIVER_HPP

#include <boost/asio.hpp>

#include "audio_config.hpp"
#include "bluetooth-serial-port/src/BTSerialPortBinding.h"
#include "bluetooth-serial-port/src/BluetoothException.h"
#include "bluetooth-serial-port/src/DeviceINQ.h"

/**
 * @class Receiver
 * @brief Abstract base class for receiving data.
 */
class Receiver {
  public:
    /**
     * @brief Destructor.
     */
    virtual ~Receiver() {}

    /**
     * @brief Constructs a Receiver object.
     * @param id The identifier for the receiver.
     * @param handle_receive_cb The callback function for handling received
     * data.
     */
    explicit Receiver(std::function<void(char buffer[], size_t recv_bytes)>
                          handle_receive_cb) {
        handle_receive_cb_ = handle_receive_cb;
    }

    /**
     * @brief Starts the receiver.
     */
    virtual void start() = 0;

    /**
     * @brief Stops the receiver.
     */
    virtual void stop() = 0;

  protected:
    std::function<void(char buffer[], size_t recv_bytes)>
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
    UDPReceiver& operator=(const UDPReceiver&&) = delete;

    /**
     * @brief Constructs a UDPReceiver object.
     * @param port The port number to listen on.
     * @param handle_receive_cb Callback function for handling received data.
     */
    explicit UDPReceiver(
        int port, std::function<void(char buffer[], size_t recv_bytes)>
                      handle_receive_cb);

    /**
     * @brief Destroys the UDPReceiver object.
     */
    ~UDPReceiver();

    /**
     * @brief Starts the UDP receiver.
     */
    virtual void start() override;

    /**
     * @brief Stops the UDP receiver.
     */
    virtual void stop() override;

    /**
     * @brief Gets the port number that the receiver is listening on.
     * @return The port number.
     */
    const int& get_port() const;

    /**
     * @brief Gets the remote endpoint from which data is received.
     * @return The remote endpoint.
     */
    const boost::asio::ip::udp::endpoint& get_remote_endpoint() const;

  private:
    /**
     * Check if a given port number is valid.
     * @param port The port number to check.
     * @return True if the port number is valid, false otherwise.
     */
    static bool is_valid_port(int port);

    /**
     * @brief Wrapper function for handling received data.
     * @param error The error code.
     * @param recv_bytes The number of received bytes.
     */
    void handle_receive_wrapper(const boost::system::error_code& error,
                                size_t recv_bytes);

    /**
     * @brief Waits for data to be received.
     */
    void wait();

  private:
    const int port_; /**< The port number to listen on. */
    boost::asio::io_service
        io_service_; /**< The IO service for asynchronous operations. */
    boost::asio::ip::udp::socket
        socket_; /**< The UDP socket for receiving data. */
    std::array<char, buffer_size>
        recv_buf_; /**< The receive buffer for incoming data. */
    boost::asio::ip::udp::endpoint remote_endpoint_; /**< The remote endpoint
                                           from which data is received. */
    std::thread thread_; /**< The thread for running the IO service. */
};

/**
 * @class BluetoothReceiver
 * @brief Class for receiving data over bluetooth.
 */
class BluetoothReceiver : public Receiver {
  public:
    BluetoothReceiver(const BluetoothReceiver&) = delete;
    BluetoothReceiver(const BluetoothReceiver&&) = delete;

    BluetoothReceiver& operator=(const BluetoothReceiver&) = delete;
    BluetoothReceiver& operator=(const BluetoothReceiver&&) = delete;

    /**
     * @brief Constructs a BluetoothReceiver object.
     * @param port The port number to listen on.
     * @param handle_receive_cb Callback function for handling received data.
     */
    explicit BluetoothReceiver(
        int port, std::function<void(char buffer[], size_t recv_bytes)>
                      handle_receive_cb);

    /**
     * @brief Destroys the BluetoothReceiver object.
     */
    ~BluetoothReceiver();

    /**
     * @brief Starts the BluetoothReceiver.
     */
    virtual void start() override;

    /**
     * @brief Stops the BluetoothReceiver.
     */
    virtual void stop() override;

    /**
     * @brief Gets the port number that the receiver is listening on.
     * @return The port number.
     */
    const int& get_port() const;

  private:
    const int port_;            /**< The port number to listen on. */
    std::atomic<bool> polling_; /**< Whether the polling thread is running. */
    std::thread thread_;        /**< Polling thread for receiving data. */
    std::unique_ptr<DeviceINQ> device_inq_; /**< The device inquirer. */
    std::unique_ptr<BTSerialPortBinding>
        binding_; /**< The serial port binding. */
};

#endif  // RECEIVER_HPP
