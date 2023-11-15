/**
 * @file sender.hpp
 * @brief The declarations of the Sender interface and its implementations.
 */

#ifndef SENDER_HPP
#define SENDER_HPP

#include <boost/asio.hpp>

#include "bluetooth-serial-port/src/BTSerialPortBinding.h"
#include "bluetooth-serial-port/src/BluetoothException.h"
#include "bluetooth-serial-port/src/DeviceINQ.h"

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
    virtual void send(const std::string& msg) = 0;
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
    virtual void send(const std::string& msg) override;

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
     * @param mac_address The MAC address to send the data to.
     * @param port The port number to send the data to.
     */
    explicit BluetoothSender(const std::string& mac_address);

    /**
     * @brief Destroys the BluetoothSender object.
     */
    ~BluetoothSender();

    /**
     * @brief Sends the provided message over UDP.
     * @param msg The message to be sent.
     */
    virtual void send(const std::string& msg) override;

  private:
    /**
     * Check if a given MAC address is valid.
     * @param mac_address The MAC address to check.
     * @return True if the MAC address is valid, false otherwise.
     */
    static bool is_valid_mac(const std::string& mac_address);

    const std::string mac_address_; /**< The MAC address to send the data to. */
    int channel_; /**< The channel to send the data on. */
    std::unique_ptr<DeviceINQ> device_inq_; /**< The device inquirer. */
    std::unique_ptr<BTSerialPortBinding>
        binding_; /**< The serial port binding. */
};

#endif  // SENDER_HPP
