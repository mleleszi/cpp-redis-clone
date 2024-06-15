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

    /**
     * Removes expired keys from the data store.
     *
     * Implementation of the Redis expiry algorithm:
     *   - Select 20 random keys from the data store.
     *   - Delete the keys that have expired.
     *   - If more than 25% of the sampled keys are expired, continue the process in the next cycle.
     *
     * @return The number of expired keys that were removed.
     */
    int removeExpiredKeys();

    /**
     * Starts a background thread to invoke DataStore::removeExpiredKeys() every 100 milliseconds.
     */
    void startExpiryDaemon();


private:
    std::unordered_map<std::string, Entry> store;
    std::mutex mtx;

    std::vector<std::string> getRandomKeys(int n);
};