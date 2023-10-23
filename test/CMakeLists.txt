file(GLOB TEST_FILES *.cpp)

add_executable(UnitTests ${TEST_FILES})

target_include_directories(UnitTests PUBLIC ./)

target_link_libraries(UnitTests
    PRIVATE
        Catch2::Catch2
        Catch2::Catch2WithMain
        pthread
        asound
        portaudio_static
        Boost::log
        Boost::program_options)

catch_discover_tests(UnitTests)
