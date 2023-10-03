#ifndef CONNECTOR_HPP
#define CONNECTOR_HPP

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/log/trivial.hpp>
#include <string>

#include "audio_config.hpp"

constexpr const unsigned int BUFFER_SIZE = 8192;

using namespace boost::asio;

class Sender {
  public:
    virtual inline void send(const std::string_view& msg) = 0;
};

class UDPSender : public Sender {
  public:
    UDPSender(const UDPSender&) = delete;
    UDPSender(const UDPSender&&) = delete;
    void operator=(const UDPSender&) = delete;

    explicit UDPSender(const std::string& ip, int port)
        : ip_(ip),
          port_(port),
          io_service_(io_service()),
          socket_(ip::udp::socket(io_service_)) {
        socket_.open(ip::udp::v4());

        BOOST_LOG_TRIVIAL(info) << "UDPSender started";
    }

    ~UDPSender() {
        socket_.close();

        BOOST_LOG_TRIVIAL(info) << "UDPSender stopped";
    }

    virtual inline void send(const std::string_view& msg) override {
        socket_.send_to(buffer(msg), ip::udp::endpoint(
                                         ip::address::from_string(ip_), port_));
    }

  private:
    const std::string ip_;
    const int port_;
    io_service io_service_;
    ip::udp::socket socket_;
};

class Receiver {
  public:
    Receiver(const std::string& id,
             std::function<void(boost::array<char, BUFFER_SIZE> buffer,
                                size_t recv_bytes)>
                 handle_receive_cb) {}

    virtual inline void start() = 0;

    virtual inline void stop() = 0;
};

class UDPReceiver : public Receiver {
  public:
    UDPReceiver(const UDPReceiver&) = delete;
    UDPReceiver(const UDPReceiver&&) = delete;
    void operator=(const UDPReceiver&) = delete;

    explicit UDPReceiver(
        int port, std::function<void(boost::array<char, BUFFER_SIZE> buffer,
                                     size_t recv_bytes)>
                      handle_receive_cb)
        : Receiver(std::to_string(port), handle_receive_cb),
          port_(port),
          handle_receive_cb_(handle_receive_cb),
          io_service_(io_service()),
          socket_(ip::udp::socket(io_service_)) {}

    ~UDPReceiver() {
        stop();
        handle_receive_cb_ = nullptr;
    }

    virtual inline void start() override {
        socket_.open(ip::udp::v4());
        socket_.bind(ip::udp::endpoint(ip::address_v4::loopback(), port_));

        thread_ = std::thread([&] {
            wait();
            io_service_.run();
        });

        BOOST_LOG_TRIVIAL(info)
            << "UDPReceiver started listening on port: " << port_;
    }

    virtual inline void stop() override {
        io_service_.stop();

        if (thread_.joinable()) {
            thread_.join();
        }

        socket_.close();

        BOOST_LOG_TRIVIAL(info)
            << "UDPReceiver stopped listening on port: " << port_;
    }

    inline const int& get_port() const { return port_; }

    inline const ip::udp::endpoint& get_remote_endpoint() const {
        return remote_endpoint_;
    }

  private:
    inline void handle_receive_wrapper(const boost::system::error_code& error,
                                       size_t recv_bytes) {
        handle_receive_cb_(recv_buf_, recv_bytes);
        wait();
    }

    inline void wait() {
        socket_.async_receive_from(
            buffer(recv_buf_), remote_endpoint_,
            boost::bind(&UDPReceiver::handle_receive_wrapper, this,
                        placeholders::error, placeholders::bytes_transferred));
    }

    const int port_;
    std::function<void(boost::array<char, BUFFER_SIZE> buffer,
                       size_t recv_bytes)>
        handle_receive_cb_;
    io_service io_service_;
    ip::udp::socket socket_;
    boost::array<char, BUFFER_SIZE> recv_buf_;
    ip::udp::endpoint remote_endpoint_;
    std::thread thread_;
};

#endif  // CONNECTOR_HPP
