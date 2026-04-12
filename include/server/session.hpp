#pragma once

#include <iostream>

#include <boost/asio.hpp>

#include "client_manager_interface.hpp"
#include "command_manager.hpp"

class Session : public std::enable_shared_from_this<Session>{ // мы должны наследоваться иначе указатель на сессию будет уничтожен, 
    boost::asio::ip::tcp::socket socket_;                     // когда не будет колбэка, а когда он придет сессия уже будет уничтожена 
    std::vector<char> buffer_;                                // так как счетчик shared_ptr обнулился.

    std::string id_;
    ClientManagerImpl& client_manager_;
    Command_manager& command_manager_;
public:
    Session(boost::asio::ip::tcp::socket socket, std::string id_client, ClientManagerImpl& client_manager, Command_manager& command_manager, size_t size_buffer = 2048);

    void start();
private:
    void do_read();
    void do_write(const std::string& answer);
    std::string get_client_info() const;
};