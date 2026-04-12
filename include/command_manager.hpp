#pragma once


#include "KVStorage.hpp"
#include "client_manager_interface.hpp"

class Command_manager{
    db_KV_storage& storage_;
    ClientManagerImpl& c_manager_;
    mutable std::shared_mutex mtx_;
public:
    Command_manager(db_KV_storage& storage, ClientManagerImpl& c_manager) : storage_(storage) , c_manager_(c_manager) {}

    std::string instruction(const std::string& message, const ClientInfo& client) const;
private:
    bool is_admin_command(const std::string& cmd) const;

    std::tuple<std::string, int, std::string> parser_cmd(const std::string& message) const;

    bool is_admin(const std::string& access) const;

    std::string format_uptime(std::chrono::steady_clock::time_point connected_since) const;

    std::string command_get(int key) const;
    std::string command_set(int key, const std::string& data) const;
    std::string command_del(int key) const;
    std::string command_clients() const;
    std::string command_log(int max_lines = 100) const;
    std::string command_admin(const std::string& password, const std::string id) const;
};