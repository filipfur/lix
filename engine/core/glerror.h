#pragma once

#include <string>
#ifdef __EMSCRIPTEN__
#include <GLES3/gl32.h>
#else
#include <GL/glew.h>
#endif

namespace lix {
void traceGLError(const std::string &className, GLuint error);
}