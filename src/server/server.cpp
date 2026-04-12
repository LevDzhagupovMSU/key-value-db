#include <iostream>
#include <memory>

#include <boost\asio.hpp> 

#include "logger.hpp"
#include "server.hpp"

Server::Server(boost::asio::io_context& io, const std::string& address, ClientManagerImpl& client_manager, 
            Command_manager& command_manager, size_t thread_count) :
                io_(io), address_(address), client_manager_(client_manager),
                command_manager_(command_manager), 
                acceptor_(io, 
                    boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(address), 
                    client_manager.get_admin_port())) {


            start_session();
            
            for(size_t i = 0; i < thread_count; i++){
                threads_.emplace_back([this] {io_.run();});
            }
            

            logger_instance.info("The server is running");

            std::cout << "The server is running on port:\n"
              << "Admin: " << address << ":" << port_ << "\n"
              << "  (" << thread_count << " threads)\n";
}


void Server::start_session(){
    acceptor_.async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket){
        if(!ec){
            auto ep = socket.remote_endpoint();
            auto id = client_manager_.add_client(ep.address().to_string(), ep.port());
            std::make_shared<Session>(std::move(socket), id, client_manager_, command_manager_)->start();
        }
        start_session();
    });
}

Server::~Server(){
    io_.stop();                 
    for (auto& t : threads_) {
        if (t.joinable()) {
            t.join();             
        }
    }
}