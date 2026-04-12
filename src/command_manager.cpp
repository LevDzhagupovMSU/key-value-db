#include <cctype>

#include "command_manager.hpp"
#include "logger.hpp"

std::string Command_manager::instruction(const std::string& message, const ClientInfo& client) const{
    std::shared_lock lock(mtx_);

    auto [cmd, key, data] = parser_cmd(message);
    std::string access = client.access_;
    std::string client_id = client.id_;
    if (cmd.empty()) {
        logger_instance.info("Unknown command from client: " + client_id + " -> " + message);
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

std::tuple<std::string, int, std::string> Command_manager::parser_cmd(const std::string& message) const{
    std::string cmd;
    int key = -1;
    std::string data;

    if(message.empty()) return std::tuple<std::string, int, std::string>(cmd, key, data);

    auto first_space = message.find(' ');
    cmd = message.substr(0, first_space);

    for (auto& c : cmd) c = toupper(c);

    while (!cmd.empty() && (cmd.back() == '\r' || cmd.back() == '\n' || 
    cmd.back() == '\t' || cmd.back() == ' ')) {
        cmd.pop_back();
    }  

    if(cmd == "CLIENTS" || cmd == "LOG") return std::tuple<std::string, int, std::string>(cmd, key, data);
    if (first_space == std::string::npos) {
        logger_instance.warning("Missing key for command: " + cmd);
        return {"", -1, ""};
    }

    if(cmd == "ADMIN"){
        size_t password_start = first_space + 1;

        std::string password = message.substr(password_start);
        password.pop_back();
        return {cmd, -1, password};
    }

    size_t key_start = first_space + 1;
    size_t key_end = message.find(' ', key_start);
    if (key_end == std::string::npos) key_end = message.size();
    
    std::string key_str = message.substr(key_start, key_end - key_start);
    try {
        key = std::stoi(key_str);
    } catch (const std::invalid_argument& e) {
        logger_instance.warning("Invalid key format: " + key_str);
        return {"", -1, ""};
    } catch (const std::out_of_range& e) {
        logger_instance.warning("Key out of range: " + key_str);
        return {"", -1, ""};
    }

    if(cmd == "DEL") return std::tuple<std::string, int, std::string>(cmd, key, data);

    if (cmd == "GET" || cmd == "SET") {
        if (key_end >= message.size()) {
            if (cmd == "SET") {
                logger_instance.warning("Missing value for SET command");
                return {"", -1, ""};
            }
            return {cmd, key, ""}; 
        }
    }

    size_t value_start = key_end + 1;
    data = message.substr(value_start);
    
    if (!data.empty() && data.back() == '\n') data.pop_back();
    if (!data.empty() && data.back() == '\r') data.pop_back();

    return std::tuple<std::string, int, std::string>(cmd, key, data); 
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