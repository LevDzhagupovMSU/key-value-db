cmake_minimum_required(VERSION 3.10)
project(KVServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#путь к boost
set(BOOST_INCLUDE_DIR "C:/boost")

add_executable(kv_server
    src/main/main.cpp

    src/client_manager_interface.cpp
    src/command_manager.cpp
    src/KVStorage.cpp
    src/admin_manager.cpp
    src/parser.cpp

    src/server/server.cpp
    src/server/session.cpp
    src/server/logger.cpp
)

target_include_directories(kv_server PRIVATE 
    ${BOOST_INCLUDE_DIR}
    include
    include/server 
)

if(WIN32)
    target_link_libraries(kv_server ws2_32 wsock32)
endif()
