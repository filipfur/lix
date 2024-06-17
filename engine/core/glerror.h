#pragma once

#include <string>

#include <GLES3/gl32.h>

namespace lix
{
    void traceGLError(const std::string& className, GLuint error);
}
