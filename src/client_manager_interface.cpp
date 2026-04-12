#include "client_manager_interface.hpp"
#include "logger.hpp"

ClientInfo::ClientInfo(std::string id, std::string address, std::string access, uint16_t port,
        std::chrono::steady_clock::time_point connected_since) : id_(id), address_(address), access_(access),
                                                 port_(port), connected_since_(connected_since) {}


ClientManagerImpl::ClientManagerImpl(uint16_t admin_port) : admin_port_(admin_port), adm_manager_() {}



std::string ClientManagerImpl::add_client(const std::string& address, uint16_t port){
    std::unique_lock lock(mtx_);

    auto id = get_id();
    auto now = std::chrono::steady_clock::now();
    if (port == admin_port_) {
        clients_.insert(std::make_pair(id, ClientInfo{id, address, "admin", port, now}));
        logger_instance.info("added ADMIN: " + id + " " + address + ":" + std::to_string(port));
        return id;
    }
    clients_.insert(std::make_pair(id, ClientInfo{id, address, "client", port, now}));
    logger_instance.info("added client: " + id + " " + address + ":" + std::to_string(port));
    return id;
}

void ClientManagerImpl::remove_client(const std::string& client_id){
    std::unique_lock lock(mtx_);

    auto it = clients_.find(client_id);
    if (it == clients_.end()) {
        logger_instance.warning("remove_client: client not found: " + client_id);
        return;
    }

    if (it->second.access_ == "admin") {
        logger_instance.warning("Trying to remove admin: " + client_id + " " + it->second.address_);
        return;
    }

    logger_instance.info("Remove client: " + client_id + " " + it->second.address_);
    clients_.erase(it);
}

std::vector<ClientInfo> ClientManagerImpl::get_all_clients() const {
    std::shared_lock lock(mtx_);

    std::vector<ClientInfo> res;
    res.reserve(clients_.size());
    for(const auto& pair : clients_){
        res.push_back(pair.second);
    }

    return res;
}

size_t ClientManagerImpl::client_count() const{
    std::shared_lock lock(mtx_);

    return clients_.size();
}

bool ClientManagerImpl::client_to_admin(const std::string& password, ClientInfo& client) const{
    if(adm_manager_.aut(password)){
        client.access_ = "admin";
        return true;
    }

    return false;
}

ClientInfo& ClientManagerImpl::get_client(const std::string& client_id){
    std::shared_lock lock(mtx_);

    auto it = clients_.find(client_id);
    if (it == clients_.end()) {
        logger_instance.error("get_client: client not found: " + client_id);
        throw std::out_of_range("Client not found: " + client_id);
    }
    return it->second;
}

uint16_t ClientManagerImpl::get_admin_port() const{
    return admin_port_;
}

std::string ClientManagerImpl::get_id(){
    return "client_" + std::to_string(next_id++);
}