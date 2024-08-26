#pragma once

#include <string>
#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
#include <GLES3/gl32.h>
#else
#include <GL/glew.h>
#endif

namespace lix {
void traceGLError(const std::string &className, GLuint error);
}