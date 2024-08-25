#include "glerror.h"

#include <iostream>
#include <unordered_map>

namespace {
static const std::unordered_map<GLuint, std::string> errorMessages{
    {GL_INVALID_ENUM, "GL_INVALID_ENUM"},
    {GL_INVALID_VALUE, "GL_INVALID_VALUE"},
    {GL_INVALID_OPERATION, "GL_INVALID_OPERATION"},
    {GL_INVALID_FRAMEBUFFER_OPERATION, "GL_INVALID_FRAMEBUFFER_OPERATION"},
    {GL_OUT_OF_MEMORY, "GL_OUT_OF_MEMORY"}};
}

void lix::traceGLError(const std::string &className, GLuint error) {
    if (error == GL_NO_ERROR) {
        return;
    }
    auto it = errorMessages.find(error);
    std::cerr << "Error in " << className << ": "
              << (it == errorMessages.end() ? "UNKNOWN" : it->second)
              << std::endl;
}