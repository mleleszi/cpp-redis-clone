#include "datastore.h"
#include "gtest/gtest.h"
#include <chrono>
#include <thread>

TEST(DataStoreTests, GetWithoutExpiry) {
    DataStore store;
    store.set("key", "val");

    auto res = store.get("key");
    ASSERT_EQ(res, "val");
}

TEST(DataStoreTests, GetExpired) {
    DataStore store;
    store.setWithExpiry("key", "val", std::chrono::system_clock::now() + std::chrono::milliseconds(100));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto res = store.get("key");
    ASSERT_FALSE(res.has_value());
}

TEST(DataStoreTests, GetNotExpired) {
    DataStore store;
    store.setWithExpiry("key", "val", std::chrono::system_clock::now() + std::chrono::milliseconds(200));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto res = store.get("key");
    ASSERT_EQ(res, "val");
}

TEST(DataStoreTests, RemoveExpiredKeys) {
    DataStore store;
    store.setWithExpiry("key0", "val0", std::chrono::system_clock::now() + std::chrono::milliseconds(1000));
    store.setWithExpiry("key1", "val1", std::chrono::system_clock::now() + std::chrono::milliseconds(200));
    store.setWithExpiry("key2", "val2", std::chrono::system_clock::now() + std::chrono::milliseconds(300));
    store.set("key3", "val3");

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    int deletedCount = store.removeExpiredKeys();
    ASSERT_EQ(deletedCount, 2);

    auto val1 = store.get("key1");
    ASSERT_FALSE(val1.has_value());

    auto val2 = store.get("key2");
    ASSERT_FALSE(val2.has_value());
}

TEST(DataStoreTests, ExpiryDaemon) {
    DataStore store;
    store.startExpiryDaemon();

    store.setWithExpiry("key0", "val0", std::chrono::system_clock::now() + std::chrono::milliseconds(1000));
    store.setWithExpiry("key1", "val1", std::chrono::system_clock::now() + std::chrono::milliseconds(200));
    store.setWithExpiry("key2", "val2", std::chrono::system_clock::now() + std::chrono::milliseconds(300));
    store.set("key3", "val3");

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto count = store.count();
    ASSERT_EQ(count, 2);

    auto val1 = store.get("key1");
    ASSERT_FALSE(val1.has_value());

    auto val2 = store.get("key2");
    ASSERT_FALSE(val2.has_value());
}