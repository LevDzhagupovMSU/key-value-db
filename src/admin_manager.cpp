#include <sstream> 
#include <iomanip>   

#include "admin_manager.hpp"

Admin_manager::Admin_manager(){
    admin_password_hash = my_hash("1234");
}

bool Admin_manager::aut(const std::string& password) const{
    if(admin_password_hash == my_hash(password)){
        return true;
    }
    return false;
}

std::string Admin_manager::my_hash(const std::string& password) const{
    size_t hash = 2026;
    for(char c : password){
        hash = ((hash << 5) + hash) + c;
    }

    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash;

    return ss.str();
}