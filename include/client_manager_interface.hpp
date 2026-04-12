#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <shared_mutex>
#include <atomic>

#include "admin_manager.hpp"

struct ClientInfo{
    std::string id_;
    std::string address_;
    std::string access_;

    std::chrono::steady_clock::time_point connected_since_;

    uint16_t port_;

    ClientInfo(std::string id, std::string address, std::string access, uint16_t port,
        std::chrono::steady_clock::time_point connected_since);
};

class IClientManager{
    public:
    virtual ~IClientManager() = default;
    
    virtual std::string add_client(const std::string& address, uint16_t port) = 0;
    virtual void remove_client(const std::string& client_id) = 0;
    virtual std::vector<ClientInfo> get_all_clients() const = 0;
    virtual size_t client_count() const = 0;
};

class ClientManagerImpl : public IClientManager{
    mutable std::shared_mutex mtx_;
    std::unordered_map<std::string, ClientInfo> clients_;
    Admin_manager adm_manager_;
    std::atomic<size_t> next_id{0};
    uint16_t admin_port_;

public:
    explicit ClientManagerImpl(uint16_t admin_port = 8080);

    std::string add_client(const std::string& address, uint16_t port) override;
    void remove_client(const std::string& client_id) override;
    std::vector<ClientInfo> get_all_clients() const override;
    size_t client_count() const override;
    bool client_to_admin(const std::string& password, ClientInfo& client) const;

    ClientInfo& get_client(const std::string& client_id);
    uint16_t get_admin_port() const;
private:
    std::string get_id();
};