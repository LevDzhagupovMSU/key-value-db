#include <cctype>

#include "command_manager.hpp"
#include "logger.hpp"

Command_manager::Command_manager(db_KV_storage& storage, ClientManagerImpl& c_manager) : 
                            storage_(storage) , c_manager_(c_manager) {}

std::string Command_manager::instruction(const std::string& message, const ClientInfo& client) const{
    std::shared_lock lock(mtx_);

    std::string clean_message = message;
    clean_message.erase(std::remove(clean_message.begin(), clean_message.end(), '\r'), clean_message.end());
    clean_message.erase(std::remove(clean_message.begin(), clean_message.end(), '\n'), clean_message.end());

    auto [cmd, key, data] = c_parser_.parser_cmd(clean_message);

    std::string access = client.access_;
    std::string client_id = client.id_;

    if (cmd.empty()) {
        logger_instance.info("Unknown command from client: " + client_id + " -> " + clean_message);
        return "ERR unknown command\r\n";
    }
    if (is_admin_command(cmd) && !is_admin(access)) {
        logger_instance.warning("Admin command denied for client: " + client_id + " (" + cmd + ")");
        return "ERR permission denied\r\n";
    }
    if (data.empty()) {
        logger_instance.info("Client " + client_id + " -> " + cmd + " " + std::to_string(key));
    } else {
        if(cmd != "ADMIN")
            logger_instance.info("Client " + client_id + " -> " + cmd + " " + std::to_string(key) + "=" + data);
    }

    if(cmd == "GET"){
        return command_get(key);
    }

    else if(cmd == "SET"){
        return command_set(key, data);
    }

    else if(cmd == "DEL"){
        return command_del(key);
    }

    else if(cmd == "CLIENTS"){
        return command_clients();
    }

    else if(cmd == "LOG"){
        return command_log();
    }

    else if(cmd == "ADMIN"){
        return command_admin(data, client.id_);
    }

    logger_instance.error("Unhandled command: " + cmd + " from " + client_id);
    return "ERR unknown command\r\n";
}

std::string Command_manager::format_uptime(std::chrono::steady_clock::time_point connected_since) const {
    auto now = std::chrono::steady_clock::now();
    auto duration = now - connected_since;
    
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    
    if (seconds < 60) {
        return std::to_string(seconds) + "s";
    }
    
    auto minutes = seconds / 60;
    seconds %= 60;
    
    if (minutes < 60) {
        return std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
    }
    
    auto hours = minutes / 60;
    minutes %= 60;
    
    return std::to_string(hours) + "h " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
}

bool Command_manager::is_admin_command(const std::string& cmd) const{
    if(cmd == "CLIENTS" || cmd == "LOG") return true;
    return false;
}

bool Command_manager::is_admin(const std::string& access) const{
    if(access == "admin") return true;
    return false;
}


std::string Command_manager::command_get(int key) const{
    auto value = storage_.get(key);

    if(value.has_value()){
        return "+OK " + value.value() + "\r\n";
    }
    logger_instance.warning("Try to command_get: No key " + std::to_string(key));
    return "-ERR key not found\r\n";
}

std::string Command_manager::command_set(int key, const std::string& data) const{
    storage_.set(key, data);
    return "+OK\r\n";
}

std::string Command_manager::command_del(int key) const{
    storage_.del(key);
    return "+OK\r\n";
}

std::string Command_manager::command_clients() const{
    auto clients = c_manager_.get_all_clients();
    if (clients.empty()) {
        return "+OK 0 clients connected\r\n";
    }
    std::string result = "+OK " + std::to_string(clients.size()) + " clients and admins connected:\r\n";

    for (const auto& client : clients) {
        std::string uptime = format_uptime(client.connected_since_);
        if(client.access_ == "admin"){
            result += "  " + client.id_ + " -> ADMIN " + " | " + client.address_ + " | connected: " + uptime + " ago\r\n";
            continue;
        }
        result += "  " + client.id_ + " -> CLIENT" + " | " + client.address_ + " | connected: " + uptime + " ago\r\n";
    }
    
    return result;
}

std::string Command_manager::command_log(int max_lines) const{
    std::ifstream file(logger_instance.get_log_file());

    if (!file.is_open()) {
        return "-ERR Cannot open log file\r\n";
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    
    file.close();
    
    int start = std::max(0, static_cast<int>(lines.size()) - max_lines);
    
    std::string res = "+OK Logs (last " + std::to_string(lines.size() - start) + " lines):\r\n";
    for (int i = start; i < lines.size(); i++) {
        res += "  " + lines[i] + "\r\n";
    }

    return res;
}

std::string Command_manager::command_admin(const std::string& password , const std::string id) const{
    auto& client = c_manager_.get_client(id);

    if(client.access_ == "admin"){
        logger_instance.info("already administrator access: " + client.id_);
        return "+OK already administrator\r\n";
    }

    if (c_manager_.client_to_admin(password, client)){
        logger_instance.info("New administrator: " + client.id_);
        return "+OK administrator access added\r\n";
    }

    logger_instance.warning("Wrong password: " + client.id_);
    return "Wrong password";
}
