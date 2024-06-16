#include "glvertexarraybuffer.h"

#include <cassert>
#include <algorithm>

lix::VertexArrayBuffer::VertexArrayBuffer(GLenum usage,
            const lix::Attributes& attributes,
            GLuint byteLength,
            GLuint componentSize,
            void* data,
            GLuint layoutOffset,
            GLuint attribDivisor,
            GLuint componentType)
    : lix::Buffer{GL_ARRAY_BUFFER, usage},
    _attributes{attributes},
    _layoutOffset{layoutOffset},
    _attribDivisor{attribDivisor},
    _componentType{componentType}
{
    this->bind(); // call to virtual in ctor
    if(byteLength > 0)
    {
        this->bufferData(byteLength, componentSize, data);
    }
    linkAttributes();
    glEnableVertexAttribArray(0);
}

lix::VertexArrayBuffer::VertexArrayBuffer(GLenum usage,
            const lix::Attributes& attributes,
            GLuint byteLength,
            GLuint componentSize,
            const void* data,
            GLuint layoutOffset,
            GLuint attribDivisor,
            GLuint componentType)
    : lix::Buffer{GL_ARRAY_BUFFER, usage},
    _attributes{attributes},
    _layoutOffset{layoutOffset},
    _attribDivisor{attribDivisor},
    _componentType{componentType}
{
    this->bind(); // call to virtual in ctor
    if(byteLength > 0)
    {
        this->bufferData(byteLength, componentSize, data);
    }
    linkAttributes();
    glEnableVertexAttribArray(0);
}

lix::VertexArrayBuffer::VertexArrayBuffer(GLenum usage,
    const lix::Attributes& attributes,
    const std::vector<GLfloat>& vertices,
    GLuint layoutOffset,
    GLuint attribDivisor) : lix::Buffer{GL_ARRAY_BUFFER, usage},
    _attributes{attributes},
    _layoutOffset{layoutOffset},
    _attribDivisor{attribDivisor},
    _componentType{GL_FLOAT}
{
    this->bind();
    if(vertices.size() > 0)
    {
        this->bufferData(vertices);
    }
    linkAttributes();
    glEnableVertexAttribArray(0);
}

lix::VertexArrayBuffer::VertexArrayBuffer(const VertexArrayBuffer& other) : Buffer{other},
    _attributes{other._attributes},
    _layoutOffset{other._layoutOffset},
    _attribDivisor{other._attribDivisor},
    _componentType{other._componentType}
{
    this->bind();
    linkAttributes();
    glEnableVertexAttribArray(0);
}

lix::VertexArrayBuffer::~VertexArrayBuffer() noexcept
{
    _attributes.clear();
}

void lix::VertexArrayBuffer::linkAttributes()
{
    // enum Attribute { FLOAT, VEC2, VEC3, VEC4, UVEC4, MAT3, MAT4 };
    static lix::AttributePointer attributePointers[] = {
        lix::AttributePointer{GL_FLOAT, 1, 1, 4},
        lix::AttributePointer{GL_FLOAT, 2, 1, 8},
        lix::AttributePointer{GL_FLOAT, 3, 1, 12},
        lix::AttributePointer{GL_FLOAT, 4, 1, 16},
        lix::AttributePointer{GL_UNSIGNED_BYTE, 4, 1, 4},
        lix::AttributePointer{GL_FLOAT, 3, 3, 12},
        lix::AttributePointer{GL_FLOAT, 4, 4, 16}
    };
    GLuint stride = 0;
    GLsizei offset = 0;
    std::for_each(_attributes.begin(), _attributes.end(), [&stride](const Attribute& attrib){
        stride += attributePointers[attrib].size * attributePointers[attrib].elements;
    });
    int i{0};
    for(const Attribute& attrib : _attributes)
    {
        const AttributePointer& attribPtr = attributePointers[attrib];

        for(int j{0}; j < static_cast<int>(attribPtr.elements); ++j)
        {
            glEnableVertexAttribArray(i + _layoutOffset);
            if(_componentType == GL_FLOAT)
            {
                glVertexAttribPointer(i + _layoutOffset, attribPtr.components,
                    attribPtr.componentType, GL_FALSE, stride, (void*)(intptr_t) offset);
            }
            else
            {
                glVertexAttribIPointer(i + _layoutOffset, attribPtr.components,
                    attribPtr.componentType, stride, (void*)(intptr_t) offset);
            }
            if(_attribDivisor > 0)
            {
                glVertexAttribDivisor(i + _layoutOffset, _attribDivisor);
            }

            offset += attribPtr.size;
            _components += attribPtr.components;
            ++i;
        }
    }
    _layouts = i;
}

GLuint lix::VertexArrayBuffer::layoutOffset() const { return _layoutOffset; }
GLuint lix::VertexArrayBuffer::attribDivisor() const { return _attribDivisor; }
GLuint lix::VertexArrayBuffer::componentType() const { return _componentType; }
GLuint lix::VertexArrayBuffer::layouts() const { return _layouts; }
GLuint lix::VertexArrayBuffer::components() const { return _components; }