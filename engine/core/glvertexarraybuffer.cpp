#include "glvertexarraybuffer.h"

#include <algorithm>
#include <cassert>

static const lix::AttributePointer &
getAttributePointer(lix::Attribute attribute) {
    // enum Attribute { FLOAT, VEC2, VEC3, VEC4, UVEC4, MAT3, MAT4 };
    static const lix::AttributePointer attributePointers[] = {
        lix::AttributePointer{GL_FLOAT, 1, 1, 4, false},
        lix::AttributePointer{GL_FLOAT, 2, 1, 8, false},
        lix::AttributePointer{GL_FLOAT, 3, 1, 12, false},
        lix::AttributePointer{GL_FLOAT, 4, 1, 16, false},
        lix::AttributePointer{GL_UNSIGNED_BYTE, 4, 1, 4, false},
        lix::AttributePointer{GL_FLOAT, 3, 3, 12, false},
        lix::AttributePointer{GL_FLOAT, 4, 4, 16, false}};
    return attributePointers[attribute];
}

static GLuint countStride(const lix::Attributes &attributes) {
    GLuint stride = 0;
    std::for_each(attributes.begin(), attributes.end(),
                  [&stride](const lix::Attribute &attr) {
                      const auto &aPtr = getAttributePointer(attr);
                      stride += aPtr.size * aPtr.elements;
                  });
    return stride;
}

lix::VertexArrayBuffer::VertexArrayBuffer(GLenum usage,
                                          const lix::Attributes &attributes,
                                          void *data, GLuint byteLength,
                                          GLuint layoutOffset,
                                          GLuint attribDivisor)
    : lix::Buffer{GL_ARRAY_BUFFER, usage}, _attributes{attributes},
      _layoutOffset{layoutOffset}, _attribDivisor{attribDivisor} {
    this->bind(); // call to virtual in ctor
    this->bufferData(GL_FLOAT, byteLength, countStride(attributes), data);
    linkAttributes();
    glEnableVertexAttribArray(0);
}

lix::VertexArrayBuffer::VertexArrayBuffer(GLenum usage,
                                          const lix::Attributes &attributes,
                                          const std::vector<GLfloat> &vertices,
                                          GLuint layoutOffset,
                                          GLuint attribDivisor)
    : lix::Buffer{GL_ARRAY_BUFFER, usage}, _attributes{attributes},
      _layoutOffset{layoutOffset}, _attribDivisor{attribDivisor} {
    this->bind();
    this->bufferData(vertices);
    linkAttributes();
    glEnableVertexAttribArray(0);
}

lix::VertexArrayBuffer::VertexArrayBuffer(const VertexArrayBuffer &other)
    : Buffer{other}, _attributes{other._attributes},
      _layoutOffset{other._layoutOffset}, _attribDivisor{other._attribDivisor} {
    this->bind();
    linkAttributes();
    glEnableVertexAttribArray(0);
}

lix::VertexArrayBuffer::~VertexArrayBuffer() noexcept { _attributes.clear(); }

void lix::VertexArrayBuffer::linkAttributes() {
    GLsizei offset = 0;
    int i{0};
    GLuint stride = countStride(_attributes);
    for (const Attribute &attrib : _attributes) {
        const AttributePointer &attribPtr = getAttributePointer(attrib);

        for (int j{0}; j < static_cast<int>(attribPtr.elements); ++j) {
            glEnableVertexAttribArray(i + _layoutOffset);
            if (attribPtr.isIntegral) {
                glVertexAttribIPointer(i + _layoutOffset, attribPtr.components,
                                       attribPtr.componentType, stride,
                                       (void *)(intptr_t)offset);
            } else {
                glVertexAttribPointer(i + _layoutOffset, attribPtr.components,
                                      attribPtr.componentType, GL_FALSE, stride,
                                      (void *)(intptr_t)offset);
            }
            if (_attribDivisor > 0) {
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
GLuint lix::VertexArrayBuffer::layouts() const { return _layouts; }
GLuint lix::VertexArrayBuffer::components() const { return _components; }