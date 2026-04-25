#pragma once

#include <string>
#include <fstream>
#include <shared_mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

enum logLevel {DEBUG_ = 0, INFO_ = 1, WARNING_ = 2, ERROR_ = 3};

class logger{
    std::ofstream logfile_;
    std::string filename_;
    mutable std::shared_mutex mtx_;
public:
    void init(const std::string filename);

    std::string get_log_file() const;

    void debug(const std::string message);
    void info(const std::string message);
    void warning(const std::string message);
    void error(const std::string message);

    ~logger();
private:
    void log(const logLevel& level, const std::string& message);
    std::string get_time();
    const char* to_string(logLevel level);
};

extern logger logger_instance;
