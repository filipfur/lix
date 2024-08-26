#pragma once

#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
#include <GLES3/gl32.h>
#else
#include <GL/glew.h>
#endif

namespace lix {
class Element {
  public:
    virtual ~Element() noexcept = 0;

    GLuint id() const;

    virtual Element *bind() = 0;
    virtual Element *unbind() = 0;

  protected:
    GLuint _id;
};
} // namespace lix