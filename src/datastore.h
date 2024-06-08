#pragma once

#include <string>
#include <unordered_map>

class DataStore {
public:
    std::unordered_map<std::string, std::string> store;
};