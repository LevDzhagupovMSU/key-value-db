#include "parser.hpp"
#include "logger.hpp"

CommandParser::CommandParser() : get_pattern(R"(^GET\s+(\d+)$)"),
                                 set_pattern(R"(^SET\s+(\d+)\s+([A-Za-zА-Яа-я]+)\s+([A-Za-zА-Яа-я]+)\s+(\d{2}\.\d{2}\.\d{4})\s+([A-Za-zА-Яа-я]+)\s+([A-Za-zА-Яа-я]+)$)"), 
                                 del_pattern(R"(^DEL\s+(\d+)$)"),
                                 clients_pattern(R"(^CLIENTS$)"),
                                 log_pattern(R"(^LOG$)"),
                                 admin_pattern(R"(^ADMIN\s+([A-Za-z0-9]+)$)") {}


std::tuple<std::string, int, std::string> CommandParser::parser_cmd(const std::string& message) const{
    static const std::vector<std::pair<std::string, std::regex>> commands = {
        {"GET", get_pattern}, {"SET", set_pattern}, {"DEL", del_pattern},
        {"CLIENTS", clients_pattern}, {"LOG", log_pattern}, {"ADMIN", admin_pattern} 
    };

    std::smatch matches;

    for(const auto& [cmd_name, pattern] : commands){
        if (std::regex_match(message, matches, pattern)){
            try{
                if(cmd_name == "GET") 
                    return {cmd_name, std::stoi(matches[1]), ""};

                else if(cmd_name == "SET"){
                    std::string name = matches[2];     
                    std::string surname = matches[3];   
                    std::string date = matches[4];      
                    std::string city = matches[5];     
                    std::string status = matches[6];    
                    
                    std::string data = name + " " + surname + " " + date + " " + city + " " + status;

                    return {cmd_name, std::stoi(matches[1]), data};
                }

                else if(cmd_name == "DEL"){
                    return {cmd_name, std::stoi(matches[1]), ""};
                }
                
                else if(cmd_name == "ADMIN"){
                    return {cmd_name, 0, matches[1]};
                }

                else
                    return {cmd_name, 0, ""};
            }
            catch(const std::exception& e){
                logger_instance.error("Parse error in " + cmd_name + 
                                     ": " + message + " " + e.what());
                return {"", -1, ""};
            }
        }
    }

    return {"", -1, ""};
}