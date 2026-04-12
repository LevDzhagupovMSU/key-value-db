#pragma once


#include <iostream>
#include <optional>
#include <unordered_map>
#include <shared_mutex>

class db_KV_storage{
    std::unordered_map<int, std::string> data_base_;
    mutable std::shared_mutex mtx_;
    std::string filename_;

public:
    explicit db_KV_storage(const std::string& filename);
    ~db_KV_storage();

    void set(int key, const std::string& value);
    std::optional<std::string> get(int key) const;
    void del(int key);
    bool contains(int key) const;

    size_t size() const;
    bool empty() const;

    std::string get_filename() const;
    size_t get_memory_usage() const;

    db_KV_storage(const db_KV_storage&) = delete;
    db_KV_storage& operator=(const db_KV_storage&) = delete;
    db_KV_storage(db_KV_storage&&) = delete;
    db_KV_storage& operator=(db_KV_storage&&) = delete;
private:
    void clear(bool save_or_nosave);
    void save_db() const;
    void load_db();
};