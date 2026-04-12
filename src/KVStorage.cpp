
#include "KVStorage.hpp"
#include "logger.hpp"


db_KV_storage::db_KV_storage(const std::string&  filename) : filename_(filename) { load_db(); }

db_KV_storage::~db_KV_storage(){ save_db(); }

void db_KV_storage::set(int key, const std::string& value){
    std::unique_lock lock(mtx_);

    if(data_base_.find(key) == data_base_.end()){
        data_base_[key] = value;
        logger_instance.info("Set new data: " + std::to_string(key) + " " + value);
        return;
    }
    data_base_[key] = value; // надо добавить в лог то что у нас в key лежит value и мы пареназначили неявно; 
    logger_instance.warning("Rewrite data: " + std::to_string(key));
}

std::optional<std::string> db_KV_storage::get(int key) const{
    std::shared_lock lock(mtx_);
    auto it = data_base_.find(key);

    if(it != data_base_.end()) return it->second; 

    return std::nullopt;
}

void db_KV_storage::del(int key){
    std::unique_lock lock(mtx_);

    if(data_base_.find(key) != data_base_.end()){
        data_base_.erase(key);  // если такого ключа можно в лог добавить или предупреждение или забить  на это тоже можно;
        logger_instance.info("Erased data: " + std::to_string(key));
        return;
    }
    logger_instance.warning("No key: " + std::to_string(key));
}

bool db_KV_storage::contains(int key) const{
    std::shared_lock lock(mtx_);

    if(data_base_.find(key) != data_base_.end()) return true;
    return false;
}

size_t db_KV_storage::size() const{
    std::shared_lock lock(mtx_);

    return data_base_.size();
} 

bool db_KV_storage::empty() const{
    std::shared_lock lock(mtx_);

    return data_base_.empty();
}

void db_KV_storage::clear(bool save){
    if(save){
        logger_instance.debug("DB saved");
        this->save_db();
        data_base_.clear();
        return;
    }
    logger_instance.debug("DB cleared");
    std::unique_lock lock(mtx_);
    data_base_.clear();
}

void db_KV_storage::save_db() const{
    std::shared_lock lock(mtx_);

    std::ofstream file(filename_);

    if(!file){
        logger_instance.error("Cannot open file: " + filename_);
        throw std::runtime_error("Cannot open file: " + filename_);
    }


    for(const auto& pair : data_base_){
        file << pair.first << "=" << pair.second << "\n";
    }
}

void db_KV_storage::load_db(){
    std::unique_lock lock(mtx_);

    std::ifstream file(filename_);

    if(!file){
        logger_instance.warning("No file: " + filename_);
        throw std::runtime_error("Cannot open DB file: " + filename_);
    }
    
    data_base_.clear();
    
    std::string line;

    while(std::getline(file, line)){
        auto eq_pos = line.find('=');
        if(eq_pos != std::string::npos){
            int key = std::stoi(line.substr(0, eq_pos));
            std::string val = line.substr(eq_pos + 1);
            data_base_[key] = val;
        }
    }
}

std::string db_KV_storage::get_filename() const{
    return filename_;
}

size_t db_KV_storage::get_memory_usage() const {
    std::shared_lock lock(mtx_);
    
    if (data_base_.empty()) {
        return sizeof(data_base_) + data_base_.bucket_count() * sizeof(void*);
    }
    
    constexpr size_t NODE_OVERHEAD = 32;     
    constexpr size_t STRING_OVERHEAD = 32;  
    
    size_t total = sizeof(data_base_);                       
    total += data_base_.bucket_count() * sizeof(void*);      
    total += data_base_.size() * (NODE_OVERHEAD + sizeof(int) + STRING_OVERHEAD);
    
    for (const auto& [key, value] : data_base_) {
        total += value.capacity(); 
        (void)key; 
    }
    
    return total;
}