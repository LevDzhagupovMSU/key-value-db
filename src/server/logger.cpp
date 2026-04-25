#include "logger.hpp"

logger logger_instance;

void logger::init(const std::string filename){
    filename_ = filename;
    std::unique_lock<std::shared_mutex> lock(mtx_);
    logfile_.open(filename, std::ios::app);
    if (!logfile_.is_open()) {
            std::cerr << "No log_file"  << filename << std::endl;
    }
}

std::string logger::get_log_file() const{
    return filename_;
}

void logger::debug(const std::string messege){ log(DEBUG_, message); }

void logger::info(const std::string messege){ log(INFO_, message); }

void logger::warning(const std::string messege){ log(WARNING_, message); }

void logger::error(const std::string messege){ log(ERROR_, message); }

void logger::log(const logLevel& level, const std::string& message){
    std::string dum = "[" + get_time() + "]" + "[" + to_string(level) + "]" + ": " + message;

    std::unique_lock<std::shared_mutex> lock(mtx_);

    if(logfile_){
        logfile_ << dum << std::endl;
        logfile_.flush();
    }
}

const char* logger::to_string(logLevel level){
    switch(level) {
        case DEBUG_:   return "DEBUG";
        case INFO_:    return "INFO";
        case WARNING_: return "WARNING";
        case ERROR_:   return "ERROR";
        default:      return "UNKNOWN";
    }
}


std::string logger::get_time() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

logger::~logger(){
    std::unique_lock<std::shared_mutex> lock(mtx_);

    logfile_.close();
}
