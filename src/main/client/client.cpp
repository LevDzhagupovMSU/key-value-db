#include <iostream>
#include <fstream>
#include <vector>
#include <winsock2.h>
#include <windows.h>

#include <boost\asio.hpp>

int main(){
    SetConsoleOutputCP(CP_UTF8);
    
    boost::asio::io_context io;

    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::make_address("127.0.0.1"), 8080);
    boost::asio::ip::tcp::socket socket(io);
    try{
        socket.connect(ep);

    }catch(const std::exception& e){
        std::cerr << "Errore connection: " << e.what() << std::endl;
        throw;
    }

    std::string message;
    std::cout << ">";
    while(std::getline(std::cin, message)){
        if(message == "quit") break;
    
        message += "\n";
        
        socket.write_some(boost::asio::buffer(message));
        boost::system::error_code ec;

        std::vector<char> buffer(4096);
        size_t bytes = socket.read_some(boost::asio::buffer(buffer), ec);
        if(!ec){
            std::string ans(buffer.data(), bytes);
            std::cout << "Server answer: " << ans << std::endl;
        }
        else{
            std::cout << ec.what() << std::endl;
            break;
        }
        std::cout << ">";
        std::cout.flush();
    }
    return 0;
}