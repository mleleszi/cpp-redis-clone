#pragma once

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

class DataStore {

public:
    std::optional<std::string> get(const std::string &key) {
        std::lock_guard<std::mutex> lock(mtx);
        if (store.find(key) == store.end()) return {};

        return store[key];
    }

    void set(const std::string &key, const std::string &val) {
        std::lock_guard<std::mutex> lock(mtx);
        store[key] = val;
    }

    bool exists(const std::string &key) {
        std::lock_guard<std::mutex> lock(mtx);
        return store.contains(key);
    }


private:
    std::unordered_map<std::string, std::string> store;
    std::mutex mtx;
};