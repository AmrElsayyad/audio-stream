file(GLOB TEST_FILES *.cpp)

add_executable(UnitTests ${TEST_FILES})

target_include_directories(UnitTests
                           PUBLIC ./ ${CMAKE_SOURCE_DIR}/src/audio_stream)

if(WIN32)
  target_link_libraries(UnitTests PRIVATE ws2_32)
endif()

target_link_libraries(UnitTests PRIVATE audiostream GTest::gtest_main)

add_test(UnitTests UnitTests)
