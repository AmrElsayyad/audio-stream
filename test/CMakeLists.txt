file(GLOB TEST_FILES *.cpp)

add_executable(UnitTests ${TEST_FILES} ${BOOST_ASIO_BLUETOOTH_SRC_FILES})

target_include_directories(UnitTests PUBLIC ./ ../src)

if(WIN32)
  target_link_libraries(UnitTests PRIVATE ws2_32)
endif()

target_link_libraries(
  UnitTests PRIVATE GTest::gtest_main Boost::log Boost::program_options
                    portaudio_static boost_asio_bluetooth)

add_test(UnitTests UnitTests)
