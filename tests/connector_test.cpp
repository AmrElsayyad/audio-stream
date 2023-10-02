#include "../src/connector.hpp"

#include <gtest/gtest.h>

TEST(UDPTest, test_send_and_receive) {
    std::string received_message{"Hello, World!"};
    UDPReceiver receiver = UDPReceiver(12345);
    receiver.start();
    std::string message{"Hello, World!"};
    UDPSender("127.0.0.1", 12345).send(message);

    // Give the receiver some time to receive the message.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    receiver.stop();

    EXPECT_EQ(message, received_message);
}
