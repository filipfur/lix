#pragma once

#include "common.h"

namespace shaderproc {
void procShaders(fs::path shaderDir, fs::path outputDir,
                 const std::string &versionOverride);
}