#pragma once

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

namespace common {
long fileSize(const fs::path &filepath);
void log(const std::string &msg);
std::string variableName(std::string name);
std::string floatToString(float f);
void exportBytes(const unsigned char *bytes, size_t length, std::ofstream &ofs);
} // namespace common