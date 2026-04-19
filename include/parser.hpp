#pragma once

#include <iostream>
#include <regex>

class CommandParser{
    std::regex get_pattern;
    std::regex set_pattern;
    std::regex del_pattern;
    std::regex clients_pattern;
    std::regex log_pattern;
    std::regex admin_pattern;
public:
    CommandParser();

    std::tuple<std::string, int, std::string> parser_cmd(const std::string& message) const;
};