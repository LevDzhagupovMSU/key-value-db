#include <vector>

#include "session.hpp"
#include "logger.hpp"


Session::Session(boost::asio::ip::tcp::socket socket, std::string id_client, 
    ClientManagerImpl& client_manager, Command_manager& command_manager, size_t size_buffer) :  
                                                                            socket_(std::move(socket)), 
                                                                            buffer_(size_buffer),
                                                                            id_(std::move(id_client)),
                                                                            client_manager_ (client_manager),
                                                                            command_manager_(command_manager) {}

void Session::start(){
    do_read();
}

void Session::do_read(){
    auto self = shared_from_this();

    socket_.async_read_some(boost::asio::buffer(buffer_), 
    [this, self](boost::system::error_code ec, size_t length){
        if(!ec){
            std::string answer = command_manager_.instruction(std::string(buffer_.data(), length), client_manager_.get_client(id_));
            do_write(answer);
        }
        else{
            if (ec == boost::asio::error::eof) {
                logger_instance.info("Client " + get_client_info() + " disconnected normally");
            } else {
                logger_instance.error("Failed to read from client " + get_client_info() + 
                          ": " + ec.message());
            }
            try {
                client_manager_.remove_client(id_);
            } catch (...) {}
            boost::system::error_code ignored;
            socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored);
            socket_.close(ignored);
        }
    });
}

void Session::do_write(const std::string& answer){
    auto self = shared_from_this();

     boost::asio::async_write(socket_, boost::asio::buffer(answer),[this, self, answer](boost::system::error_code ec, size_t bytes){
        if(!ec){
            do_read();
        }
        else{
            logger_instance.error("Failed to write answer to client " + get_client_info() + 
              ": " + ec.message()); 
        }
    });
}

std::string Session::get_client_info() const {
    try {
        if (socket_.is_open()) {
            auto endpoint = socket_.remote_endpoint();
            return endpoint.address().to_string() + ":" + std::to_string(endpoint.port());
        }
    } catch (...) {}
    return "disconnected_client";
}