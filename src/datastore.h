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
    std::optional<std::string> get(const std::string &key) {
        std::lock_guard<std::mutex> lock(mtx);
        if (store.find(key) == store.end()) return {};

        auto now = std::chrono::system_clock::now();
        auto entry = store[key];

        if (entry.expiry && entry.expiry < now) {
            store.erase(key);
            return {};
        }

        return entry.value;
    }

    void set(const std::string &key, const std::string &val) {
        std::lock_guard<std::mutex> lock(mtx);
        store[key] = {val};
    }

    void setWithExpiry(const std::string &key, const std::string &val,
                       std::chrono::time_point<std::chrono::system_clock> expiry) {
        std::lock_guard<std::mutex> lock(mtx);
        store[key] = {val, expiry};
    }

    bool exists(const std::string &key) {
        std::lock_guard<std::mutex> lock(mtx);
        return store.contains(key);
    }

    int removeExpiredKeys() {
        auto keys = getRandomKeys(20);
        auto now = std::chrono::system_clock::now();

        int deleted = 0;

        for (const auto &key: keys) {
            auto entry = store[key];
            if (entry.expiry && entry.expiry < now) {
                store.erase(key);
                ++deleted;
            }
        }

        return deleted;
    }

    void startExpiryDaemon() {
        auto backgroundThread = [this]() {
            while (true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                removeExpiredKeys();
            }
        };

        std::thread(backgroundThread).detach();
    }

    int count() {
        std::lock_guard<std::mutex> lock(mtx);
        return store.size();
    }


private:
    std::unordered_map<std::string, Entry> store;
    std::mutex mtx;

    std::vector<std::string> getRandomKeys(int n) {
        std::vector<std::string> keys;
        keys.reserve(store.size());
        for (const auto &pair: store) { keys.push_back(pair.first); }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(keys.begin(), keys.end(), gen);

        if (keys.size() > n) { keys.resize(n); }

        return keys;
    }
};