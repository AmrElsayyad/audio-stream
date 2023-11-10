file(GLOB SRC_FILES audio_stream/*.cpp)

add_library(audio_stream STATIC ${SRC_FILES})

target_link_libraries(
  audio_stream PRIVATE Boost::log Boost::program_options portaudio_static
                       boost_asio_bluetooth)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC audio_stream)

if(WIN32)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ws2_32)
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} audio_stream)
