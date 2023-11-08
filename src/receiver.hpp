/**
 * @file receiver.hpp
 * @brief The declarations of the Receiver interface and its implementations.
 */

#ifndef RECEIVER_HPP
#define RECEIVER_HPP

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>
#include <boost/thread/mutex.hpp>

#include "../boost_asio_bluetooth/wrapper.h"
#include "audio_config.hpp"

static constexpr const unsigned int BUFFER_SIZE = 8192;

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
    virtual inline void start() = 0;

    /**
     * @brief Stops the receiver.
     */
    virtual inline void stop() = 0;

  protected:
    std::function<void(uint8_t buffer[], size_t recv_bytes)>
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
        int port, std::function<void(uint8_t buffer[], size_t recv_bytes)>
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
        handle_receive_cb_(recv_buf_.data(), recv_bytes);
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

    /**
     * @brief Constructs a BluetoothReceiver object.
     * @param port The port number to listen on.
     * @param handle_receive_cb Callback function for handling received data.
     */
    explicit BluetoothReceiver(
        boost::shared_ptr<Hive> hive, int port,
        std::function<void(uint8_t buffer[], size_t recv_bytes)>
            handle_receive_cb)
        : Receiver(handle_receive_cb),
          port_(port),
          polling_(false),
          acceptor_(std::make_shared<ConnectionAcceptor>(hive).get()),
          Connection(hive) {}

    /**
     * @brief Destroys the BluetoothReceiver object.
     */
    ~BluetoothReceiver() {
        stop();
        handle_receive_cb_ = nullptr;
    }

    /**
     * @brief Starts the BluetoothReceiver.
     */
    virtual inline void start() override {
        acceptor_->Listen(port_);
        acceptor_->Accept(boost::shared_ptr<Connection>(this));
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
    virtual inline void stop() override {
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
    inline const int& get_port() const { return port_; }

  private:
    /**
     * Handles the accept event for a given MAC address and channel.
     *
     * @param mac_addr the MAC address of the accepted connection
     * @param channel the channel of the accepted connection
     */
    void OnAccept(const std::string& mac_addr, uint8_t channel) override {
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
    void OnConnect(const std::string& mac_addr, uint8_t channel) override {
        BOOST_LOG_TRIVIAL(info)
            << "[OnConnect] " << mac_addr << ":" << channel << "\n";
        acceptor_->stream_lock_.unlock();

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
        acceptor_->stream_lock_.lock();
        handle_receive_cb_(buffer.data(), buffer.size());
        acceptor_->stream_lock_.unlock();

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
        acceptor_->stream_lock_.lock();
        BOOST_LOG_TRIVIAL(error) << "[OnError] " << error << "\n";
        acceptor_->stream_lock_.unlock();
    }

    /**
     * @class ConnectionAcceptor
     * @brief Class responsible for accepting connections.
     *
     * This class is used to handle the acceptance of incoming connections.
     * It provides methods for accepting connections, handling timer events, and
     * handling errors.
     */
    class ConnectionAcceptor : public Acceptor {
      public:
        ConnectionAcceptor(boost::shared_ptr<Hive> hive) : Acceptor(hive) {}

        ~ConnectionAcceptor() {}

      private:
        /**
         * Handles the accept event for a connection.
         *
         * @param connection A shared pointer to the Connection object.
         * @param addr The address of the connection.
         * @param channel The channel of the connection.
         *
         * @return true if the accept event was handled successfully.
         */
        bool OnAccept(boost::shared_ptr<Connection> connection,
                      const std::string& addr, uint8_t channel) override {
            stream_lock_.lock();
            BOOST_LOG_TRIVIAL(info)
                << "[OnAccept] " << addr << ":" << channel << "\n";
            stream_lock_.unlock();

            return true;
        }

        /**
         * Handle the timer event.
         *
         * @param delta the time duration since the last timer event.
         */
        void OnTimer(const boost::posix_time::time_duration& delta) override {}

        /**
         * Handle an error that occurs in the program.
         *
         * @param error The error code representing the error that occurred.
         */
        void OnError(const boost::system::error_code& error) override {
            stream_lock_.lock();
            BOOST_LOG_TRIVIAL(error) << "[OnError] " << error << "\n";
            stream_lock_.unlock();
        }

      public:
        boost::mutex stream_lock_; /**< The stream lock. */
    };

  private:
    const int port_;               /**< The port number to listen on. */
    boost::shared_ptr<ConnectionAcceptor> acceptor_; /**< The acceptor. */
    std::atomic<bool> polling_; /**< Whether the polling thread is running. */
    std::thread thread_;        /**< Polling thread for receiving data. */
};

#endif  // RECEIVER_HPP
