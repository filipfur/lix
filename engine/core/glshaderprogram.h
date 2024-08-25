#pragma once

#if __EMSCRIPTEN__
#define LIX_SHADER_VERSION "#version 300 es"
#else
#define LIX_SHADER_VERSION "#version 330 core"
#endif

#include <string>
#include <unordered_map>
#include <vector>

#include "glelement.h"
#include "glm/gtc/type_ptr.hpp"

namespace lix {
class Shader {
  public:
    Shader(GLenum type);
    ~Shader() noexcept;

    void compile(const char *src);

    GLuint id() const;

  private:
    GLuint _id;
};

class ShaderProgram : public Element {
  public:
    ShaderProgram(const char *vertexSrc, const char *fragSrc);
    virtual ~ShaderProgram() noexcept;

    virtual ShaderProgram *bind() override;
    virtual ShaderProgram *unbind() override;

    GLuint loadUniform(const std::string &name);

    ShaderProgram *setUniform(const std::string &name, GLfloat f);
    ShaderProgram *setUniform(const std::string &name, GLint i);
    ShaderProgram *setUniform(const std::string &name, const glm::vec2 &vector);
    ShaderProgram *setUniform(const std::string &name,
                              const glm::ivec2 &vector);
    ShaderProgram *setUniform(const std::string &name, const glm::vec3 &vector);
    ShaderProgram *setUniform(const std::string &name,
                              const glm::ivec3 &vector);
    ShaderProgram *setUniform(const std::string &name, const glm::vec4 &vector);
    ShaderProgram *setUniform(const std::string &name, const glm::mat3 &matrix);
    ShaderProgram *setUniform(const std::string &name, const glm::mat4 &matrix);
    ShaderProgram *setUniform(const std::string &name,
                              const std::vector<glm::vec3> &vectors);

    static const GLuint INVALID_LOCATION{0xffffffff};

  private:
    bool checkStatus();
    void link();

    Shader _vertexShader;
    Shader _fragmentShader;

    std::unordered_map<std::string, GLuint> _uniforms;
};

using ShaderProgramPtr = std::shared_ptr<ShaderProgram>;
} // namespace lix