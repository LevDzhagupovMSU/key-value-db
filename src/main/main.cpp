#include <boost/asio.hpp>
#include <iostream>
#include <filesystem>

#include "logger.hpp"
#include "KVStorage.hpp"
#include "client_manager_interface.hpp"
#include "command_manager.hpp"
#include "server.hpp"

int main() {
    try {
        logger_instance.init("server.log");
        logger_instance.info("=== SERVER STARTING ===");

        db_KV_storage storage("kv.db");
        ClientManagerImpl client_manager(8080);
        Command_manager cmd_manager(storage, client_manager);

        boost::asio::io_context io_context;

        Server server(io_context, "0.0.0.0", client_manager, cmd_manager, 4);
        
        std::cout << "Server created. Press Enter to stop..." << std::endl;
        
        std::thread io_thread([&io_context]() {
            io_context.run();
        });
        
        std::cin.get();
        
        io_context.stop(); 
        io_thread.join();  
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}