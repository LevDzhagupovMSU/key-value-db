cmake_minimum_required(VERSION 3.10)
project(client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BOOST_INCLUDE_DIR "C:/boost") 

add_executable(client client.cpp)

target_include_directories(client PRIVATE ${BOOST_INCLUDE_DIR})

if(WIN32)
    target_link_libraries(client ws2_32)
endif()