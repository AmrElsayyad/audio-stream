/**
 * @file receiver.hpp
 * @brief The declarations of the Receiver interface and its implementations.
 */

#ifndef RECEIVER_HPP
#define RECEIVER_HPP

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

#include "boost_asio_bluetooth/wrapper.h"

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
    explicit Receiver(std::function<void(uint8_t buffer[], size_t recv_bytes)>
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
    std::function<void(uint8_t buffer[], size_t recv_bytes)>
        handle_receive_cb_; /**< The callback function for handling received
                               data. */
};

/**
 * @class UDPReceiver
 * @brief Class for receiving data over UDP.
 */
class UDPReceiver : public Receiver,
                    public boost::enable_shared_from_this<UDPReceiver> {
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
        int port, std::function<void(uint8_t buffer[], size_t recv_bytes)>
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
    std::array<uint8_t, BUFFER_SIZE>
        recv_buf_; /**< The receive buffer for incoming data. */
    boost::asio::ip::udp::endpoint remote_endpoint_; /**< The remote endpoint
                                           from which data is received. */
    std::thread thread_; /**< The thread for running the IO service. */
};

/**
 * @class BluetoothReceiver
 * @brief Class for receiving data over bluetooth.
 */
class BluetoothReceiver : public Receiver, public Connection {
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
        boost::shared_ptr<Hive> hive, int port,
        std::function<void(uint8_t buffer[], size_t recv_bytes)>
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
    /**
     * Handles the accept event for a given MAC address and channel.
     *
     * @param mac_addr the MAC address of the accepted connection
     * @param channel the channel of the accepted connection
     */
    void OnAccept(const std::string& mac_addr, uint8_t channel) override;

    /**
     * OnConnect function is called when a connection is established.
     *
     * @param mac_addr the MAC address of the connected device
     * @param channel the channel used for the connection
     */
    void OnConnect(const std::string& mac_addr, uint8_t channel) override;

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
    void OnRecv(std::vector<uint8_t>& buffer) override;

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
    void OnError(const boost::system::error_code& error) override;

    /**
     * @class ConnectionAcceptor
     * @brief A class that handles the accept event for a connection.
     *
     * This class inherits from the base class Acceptor and provides
     * functionality to handle the accept event for a connection. It logs the
     * address and channel of the connection and returns true to indicate that
     * the accept event was handled successfully. It also handles timer events
     * and errors by logging them.
     */
    class ConnectionAcceptor : public Acceptor {
      public:
        /**
         * @brief Constructor for ConnectionAcceptor.
         *
         * @param hive A shared pointer to a Hive object.
         */
        ConnectionAcceptor(boost::shared_ptr<Hive> hive) : Acceptor(hive) {}

        /**
         * @brief Destructor for ConnectionAcceptor.
         */
        ~ConnectionAcceptor() {}

      private:
        /**
         * @brief Handles the accept event for a connection.
         *
         * @param connection A shared pointer to the Connection object.
         * @param addr The address of the connection.
         * @param channel The channel of the connection.
         *
         * @return true if the accept event was handled successfully.
         */
        bool OnAccept(boost::shared_ptr<Connection> connection,
                      const std::string& addr, uint8_t channel) override;

        /**
         * @brief Handles the timer event.
         *
         * @param delta The time duration since the last timer event.
         */
        void OnTimer(const boost::posix_time::time_duration& delta) override {}

        /**
         * @brief Handles an error that occurs in the program.
         *
         * @param error The error code representing the error that occurred.
         */
        void OnError(const boost::system::error_code& error) override;

      public:
        boost::mutex stream_lock_; /**< The stream lock. */
    };

  private:
    const int port_; /**< The port number to listen on. */
    boost::shared_ptr<ConnectionAcceptor> acceptor_; /**< The acceptor. */
    std::atomic<bool> polling_; /**< Whether the polling thread is running. */
    std::thread thread_;        /**< Polling thread for receiving data. */
};

#endif  // RECEIVER_HPP
