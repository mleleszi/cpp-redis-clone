#include "datastore.h"

std::optional<std::string> DataStore::get(const std::string &key) {
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

void DataStore::set(const std::string &key, const std::string &val) {
    std::lock_guard<std::mutex> lock(mtx);
    store[key] = {val};
}

void DataStore::setWithExpiry(const std::string &key, const std::string &val,
                              std::chrono::time_point<std::chrono::system_clock> expiry) {
    std::lock_guard<std::mutex> lock(mtx);
    store[key] = {val, expiry};
}

bool DataStore::exists(const std::string &key) {
    std::lock_guard<std::mutex> lock(mtx);
    return store.contains(key);
}

int DataStore::count() {
    std::lock_guard<std::mutex> lock(mtx);
    return store.size();
}

int DataStore::removeExpiredKeys() {
    std::lock_guard<std::mutex> lock(mtx);
    auto keys = getRandomKeys(20);
    auto numKeys = keys.size();
    auto now = std::chrono::system_clock::now();

    int deleted = 0;

    for (const auto &key: keys) {
        auto entry = store[key];
        if (entry.expiry && entry.expiry < now) {
            store.erase(key);
            ++deleted;
        }
        if (static_cast<float>(deleted) >= 0.25f * static_cast<float>(numKeys)) break;
    }

    return deleted;
}

void DataStore::startExpiryDaemon() {
    auto backgroundThread = [this]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            removeExpiredKeys();
        }
    };

    std::thread(backgroundThread).detach();
}

std::vector<std::string> DataStore::getRandomKeys(int n) {
    std::vector<std::string> keys;
    keys.reserve(store.size());
    for (const auto &pair: store) { keys.push_back(pair.first); }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(keys.begin(), keys.end(), gen);

    if (keys.size() > n) { keys.resize(n); }

    return keys;
}
