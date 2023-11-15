file(GLOB SRC_FILES audio_stream/*.cpp)

add_library(audiostream STATIC ${SRC_FILES})

if(WIN32)
  target_link_libraries(audiostream PRIVATE ws2_32)
endif()

target_link_libraries(audiostream PRIVATE Boost::log Boost::program_options
                                           portaudio_static bluetoothserialport)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

target_link_libraries(${CMAKE_PROJECT_NAME} audiostream)
