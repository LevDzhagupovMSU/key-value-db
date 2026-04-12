#pragma once

#include <iostream>
#include <vector>

#include "client_manager_interface.hpp"
#include "command_manager.hpp"
#include "server\session.hpp"

class Server{
    boost::asio::io_context& io_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::vector<std::thread> threads_;
    std::string address_;
    ClientManagerImpl& client_manager_;
    Command_manager& command_manager_;

    uint16_t port_;

public:
    Server(boost::asio::io_context& io, const std::string& address, ClientManagerImpl& client_manager, 
           Command_manager& command_manager, size_t thread_count = 4);

    ~Server();
private:
    void start_session();
};