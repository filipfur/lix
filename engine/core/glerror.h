#pragma once

#include <string>
#include <GL/glew.h>

namespace lix
{
    void traceGLError(const std::string& className, GLuint error);
}