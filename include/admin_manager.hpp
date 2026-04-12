#pragma once


#include <iostream>

class Admin_manager{
    std::string admin_password_hash;
public:
    Admin_manager();

    bool aut(const std::string& password) const;

private:
    std::string my_hash(const std::string& password) const;
};