file(GLOB TEST_FILES *.cpp)

add_executable(UnitTests ${TEST_FILES})

target_include_directories(UnitTests PUBLIC ./ ../src)

if(WIN32)
    target_link_libraries(UnitTests PRIVATE ws2_32)
endif()

target_link_libraries(UnitTests
    PRIVATE
        GTest::gtest_main
        portaudio_static
        Boost::log
        Boost::program_options)

add_test(UnitTests UnitTests)
