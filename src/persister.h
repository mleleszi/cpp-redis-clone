#pragma once

#include "spdlog/spdlog.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "protocol.h"

class Controller;

class WriteAheadLogPersister {
public:
    WriteAheadLogPersister(const std::string &fileName);
    ~WriteAheadLogPersister();

    void writeAndFlush(const std::vector<uint8_t> &data);

    static void restoreFromFile(const std::string &fileName, Controller &controller);

private:
    std::ofstream file;
};
