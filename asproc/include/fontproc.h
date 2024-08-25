#pragma once

#include "common.h"

namespace fontproc {
void procFont(fs::path fontDir, fs::path outputDir, bool flipY);
}