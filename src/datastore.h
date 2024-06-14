#pragma once

#include <algorithm>
#include <chrono>
#include <mutex>
#include <optional>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

struct Entry {
    std::string value;
    std::optional<std::chrono::time_point<std::chrono::system_clock>> expiry;
};

class DataStore {

public:
    std::optional<std::string> get(const std::string &key);
    void set(const std::string &key, const std::string &val);
    void setWithExpiry(const std::string &key, const std::string &val,
                       std::chrono::time_point<std::chrono::system_clock> expiry);
    bool exists(const std::string &key);
    int count();
    int removeExpiredKeys();
    void startExpiryDaemon();


private:
    std::unordered_map<std::string, Entry> store;
    std::mutex mtx;

    std::vector<std::string> getRandomKeys(int n);
};