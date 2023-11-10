/**
 * @file sender.hpp
 * @brief The declarations of the Sender interface and its implementations.
 */

#ifndef SENDER_HPP
#define SENDER_HPP

#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>

#include "boost_asio_bluetooth/wrapper.h"

/**
 * @class Sender
 * @brief Abstract base class for sending data.
 */
class Sender : public boost::enable_shared_from_this<Sender> {
  public:
    /**
     * @brief Destructor.
     */
    virtual ~Sender() {}

    /**
     * @brief Sends the provided message.
     * @param msg The message to be sent.
     */
    virtual void send(const std::string_view& msg) = 0;
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
    UDPSender& operator=(const UDPSender&&) = delete;

    /**
     * @brief Constructs a UDPSender object.
     * @param ip The IP address to send the data to.
     * @param port The port number to send the data to.
     */
    explicit UDPSender(const std::string& ip, int port);

    /**
     * @brief Destroys the UDPSender object.
     */
    ~UDPSender();

    /**
     * @brief Sends the provided message over UDP.
     * @param msg The message to be sent.
     */
    virtual void send(const std::string_view& msg) override;

  private:
    /**
     * Check if a given port number is valid.
     * @param port The port number to check.
     * @return True if the port number is valid, false otherwise.
     */
    static bool is_valid_port(int port);

    /**
     * Check if a given IP address is valid.
     * @param ip The IP address to check.
     * @return True if the IP address is valid, false otherwise.
     */
    static bool is_valid_ip(const std::string& ip);

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
class BluetoothSender : public Sender {
  public:
    BluetoothSender(const BluetoothSender&) = delete;
    BluetoothSender(const BluetoothSender&&) = delete;

    BluetoothSender& operator=(const BluetoothSender&) = delete;
    BluetoothSender& operator=(const BluetoothSender&&) = delete;

    /**
     * @brief Constructs a BluetoothSender object.
     * @param ip The IP address to send the data to.
     * @param port The port number to send the data to.
     */
    explicit BluetoothSender(boost::shared_ptr<Hive> hive,
                             const std::string& mac_address, int port);

    /**
     * @brief Destroys the BluetoothSender object.
     */
    ~BluetoothSender();

    /**
     * @brief Sends the provided message over UDP.
     * @param msg The message to be sent.
     */
    virtual void send(const std::string_view& msg) override;

  private:
    /**
     * Check if a given port number is valid.
     * @param port The port number to check.
     * @return True if the port number is valid, false otherwise.
     */
    static bool is_valid_port(int port);

    /**
     * Check if a given MAC address is valid.
     * @param mac_address The MAC address to check.
     * @return True if the MAC address is valid, false otherwise.
     */
    static bool is_valid_mac(const std::string& mac_address);

    /**
     * @class BluetoothConnection
     * @brief A private nested class that extends the Connection class and
     * handles Bluetooth connection events.
     *
     * This class contains several overridden methods that handle different
     * events related to a Bluetooth connection, such as accepting a connection,
     * establishing a connection, sending data, receiving data, handling timer
     * events, and handling errors.
     */
    class BluetoothConnection : public Connection {
      private:
        /**
         * Handles the accept event for a given MAC address and channel.
         *
         * @param mac_addr The MAC address of the accepted connection.
         * @param channel The channel of the accepted connection.
         */
        void OnAccept(const std::string& mac_addr, uint8_t channel) override;

        /**
         * Handles the connect event for a given MAC address and channel.
         *
         * @param mac_addr The MAC address of the connected device.
         * @param channel The channel used for the connection.
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
         * @param buffer The std::vector buffer containing the data to be
         * processed.
         */
        void OnRecv(std::vector<uint8_t>& buffer) override;

        /**
         * Handles the timer event.
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

      public:
        /**
         * Constructs a BluetoothConnection object.
         *
         * @param hive A shared pointer to the Hive object.
         */
        BluetoothConnection(boost::shared_ptr<Hive> hive) : Connection(hive) {}

        /**
         * Destroys the BluetoothConnection object.
         */
        ~BluetoothConnection() {}

      private:
        boost::mutex stream_lock_; /**< The stream lock. */
    };

  private:
    boost::shared_ptr<BluetoothConnection> connection_; /**< The connection. */
    const std::string mac_address_; /**< The MAC address to send the data to. */
    const int port_;                /**< The port number to listen on. */
    std::atomic<bool> polling_; /**< Whether the polling thread is running. */
    std::thread thread_;        /**< Polling thread for receiving data. */
};

#endif  // SENDER_HPP
