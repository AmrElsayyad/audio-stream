file(GLOB SRC_FILES *.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_FILES})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ./)

if(WIN32)
    target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ws2_32)
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} 
    PRIVATE
        portaudio_static
        Boost::log
        Boost::program_options)
