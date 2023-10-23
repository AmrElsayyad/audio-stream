/**
 * @file connector_test.cpp
 * @brief Contains the unit tests for the UDPReceiver and UDPSender classes.
 */

#include "../src/connector.hpp"

#include <catch.hpp>

using std::array;
using std::string;
using std::chrono::milliseconds;
using std::this_thread::sleep_for;

/**
 * @brief Test case for sending and receiving data using UDP.
 */
TEST_CASE("UDP_test_send_and_receive") {
    string received_message;
    UDPReceiver receiver = UDPReceiver(
        12345, [&](array<char, BUFFER_SIZE> buf, size_t recv_bytes) {
            for (size_t i = 0; i < recv_bytes; ++i) {
                received_message += buf[i];
            }
        });

    receiver.start();

    string message{"Hello, World!"};
    UDPSender("127.0.0.1", 12345).send(message);

    // Give the receiver some time to receive the message.
    sleep_for(milliseconds(1));

    receiver.stop();

    CHECK(message == received_message);
}
