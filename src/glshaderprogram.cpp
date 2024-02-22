#include "glshaderprogram.h"

#include <vector>
#include <iostream>

lix::Shader::Shader(GLenum type)
{
    _id = glCreateShader(type);
}
 
lix::Shader::~Shader() noexcept
{
    glDeleteShader(_id);
}
 
void lix::Shader::compile(const char* src)
{
        glShaderSource(_id, 1, &src, nullptr);
        glCompileShader(_id);
        GLint status{0};
        GLint length{0};
        glGetShaderiv(_id, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
        {
            glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &length);
            std::vector<GLchar> errorLog(length);
            glGetShaderInfoLog(_id, length, &length, &errorLog[0]);
            std::string str{src};
            std::cerr << "'" << _id << "' '" << str.substr(0, 48) <<  "' compiled with error:" << std::endl;
            std::copy(errorLog.begin(), errorLog.end(), std::ostream_iterator<GLchar>(std::cout, ""));
            exit(1);
        }
}

GLuint lix::Shader::id() const { return _id; }
 
lix::ShaderProgram::ShaderProgram(const char* vertexSrc, const char* fragSrc) : _vertexShader{GL_VERTEX_SHADER}, _fragmentShader{GL_FRAGMENT_SHADER}
{
    _id = glCreateProgram();
    _vertexShader.compile(vertexSrc);
    _fragmentShader.compile(fragSrc);
    link();
}
 
lix::ShaderProgram::~ShaderProgram() noexcept
{
    glDeleteProgram(_id);
}
 
bool lix::ShaderProgram::checkStatus()
{
    GLint result = GL_FALSE;
    glGetProgramiv(_id, GL_LINK_STATUS, &result);
    int info_log_length = 1024;
    char * infoLog =  new char[info_log_length];
    if (!result)
    {
        glGetProgramInfoLog(_id, info_log_length, NULL, infoLog);
        std::cerr << "ERROR::During link of ShaderProgram\n" << infoLog
            << "\n ******************************** " << std::endl;
    }
    return result;
}
 
void lix::ShaderProgram::link()
{
    glAttachShader(_id, _vertexShader.id());
    glAttachShader(_id, _fragmentShader.id());
    glLinkProgram(_id);
    if(!checkStatus())
    {
        //std::cerr << "File: " << _vertexShader->fileName() << std::endl;
        //std::cerr << "File: " << _fragmentShader->fileName() << std::endl;
    }
}
 
lix::ShaderProgram* lix::ShaderProgram::bind()
{
    glUseProgram(_id);
    return this;
}
 
lix::ShaderProgram* lix::ShaderProgram::unbind()
{
    glUseProgram(0);
    return this;
}

GLuint lix::ShaderProgram::loadUniform(const std::string &name)
{
    GLuint id = 0;
    auto it = _uniforms.find(name);
    if (it == _uniforms.end())
    {
        id = glGetUniformLocation(_id, name.c_str());
        if(id == INVALID_LOCATION)
        {
            std::cerr << "Failed to load uniform '" << name << "' inside " << _id << std::endl;
        }
        _uniforms.emplace(name, id);
    }
    else
    {
        id = it->second;
    }
    return id;
}

lix::ShaderProgram* lix::ShaderProgram::setUniform(const std::string& name, GLfloat f)
{
    glUniform1f(loadUniform(name), f);
    return this;
}

lix::ShaderProgram* lix::ShaderProgram::setUniform(const std::string& name, GLint i)
{
    glUniform1i(loadUniform(name), i);
    return this;
}

lix::ShaderProgram* lix::ShaderProgram::setUniform(const std::string& name, const glm::vec2& vector)
{
    glUniform2fv(loadUniform(name), 1, glm::value_ptr(vector));
    return this;
}

lix::ShaderProgram* lix::ShaderProgram::setUniform(const std::string& name, const glm::ivec2& vector)
{
    glUniform2iv(loadUniform(name), 1, glm::value_ptr(vector));
    return this;
}

lix::ShaderProgram* lix::ShaderProgram::setUniform(const std::string& name, const glm::vec3& vector)
{
    glUniform3fv(loadUniform(name), 1, glm::value_ptr(vector));
    return this;
}

lix::ShaderProgram* lix::ShaderProgram::setUniform(const std::string& name, const glm::ivec3& vector)
{
    glUniform3iv(loadUniform(name), 1, glm::value_ptr(vector));
    return this;
}

lix::ShaderProgram* lix::ShaderProgram::setUniform(const std::string& name, const glm::vec4& vector)
{
    glUniform4fv(loadUniform(name), 1, glm::value_ptr(vector));
    return this;
}

lix::ShaderProgram* lix::ShaderProgram::setUniform(const std::string& name, const glm::mat3& matrix)
{
    glUniformMatrix3fv(loadUniform(name), 1, GL_FALSE, glm::value_ptr(matrix));
    return this;
}

lix::ShaderProgram* lix::ShaderProgram::setUniform(const std::string& name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(loadUniform(name), 1, GL_FALSE, glm::value_ptr(matrix));
    return this;
}

lix::ShaderProgram* lix::ShaderProgram::setUniform(const std::string& name, const std::vector<glm::vec3>& vectors)
{
    for(size_t i = 0; i < vectors.size(); ++i)
    {
        setUniform(name + "[" + std::to_string(i) + "]", vectors.at(i));
    }
    return this;
}