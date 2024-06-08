#pragma once

#include <optional>
#include <string>
#include <unordered_map>

class DataStore {

public:
    std::optional<std::string> get(const std::string &key) {
        if (store.find(key) == store.end()) return {};

        return store[key];
    }

    void set(const std::string &key, const std::string &val) { store[key] = val; }


private:
    std::unordered_map<std::string, std::string> store;
};