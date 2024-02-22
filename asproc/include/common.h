#pragma once

#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cassert>

namespace fs = std::filesystem;

namespace common
{
    long fileSize(const fs::path& filepath);
    void log(const std::string& msg);
    std::string variableName(std::string name);
    std::string floatToString(float f);
    void exportBytes(const std::vector<unsigned char>& bytes, std::ofstream& ofs);
}