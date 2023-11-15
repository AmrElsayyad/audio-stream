/**
 * @file connector_test.cpp
 * @brief Contains the unit tests for the UDPReceiver and UDPSender classes.
 */

#include <gtest/gtest.h>

#include <boost/log/trivial.hpp>

#include "audio_stream.hpp"

using std::array;
using std::string;
using std::chrono::milliseconds;
using std::this_thread::sleep_for;

/**
 * @brief Test case for sending and receiving data using UDP.
 */
TEST(UDPTest, test_send_and_receive) {
    string message{"Hello, World!"};
    string received_message;

    UDPReceiver receiver =
        UDPReceiver(12345, [&](uint8_t buf[], size_t recv_bytes) {
            for (size_t i = 0; i < recv_bytes; ++i) {
                received_message += buf[i];
            }
        });

    receiver.start();

    BOOST_LOG_TRIVIAL(info) << "Sending message: " << message;
    UDPSender("127.0.0.1", 12345).send(message);

    // Give the receiver some time to receive the message.
    sleep_for(milliseconds(1));

    BOOST_LOG_TRIVIAL(info) << "Received message: " << received_message;

    receiver.stop();

    EXPECT_EQ(message, received_message);
}
