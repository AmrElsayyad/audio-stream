file(GLOB SRC_FILES *.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ./)

target_link_libraries(${CMAKE_PROJECT_NAME} 
    PRIVATE
        pthread
        asound
        portaudio_static
        Boost::log
        Boost::program_options)
