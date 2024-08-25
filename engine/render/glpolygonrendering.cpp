#include "glpolygonrendering.h"

namespace {
void sortedPolygonToMeshData(const std::vector<glm::vec3> &points,
                             std::vector<GLfloat> &vertices,
                             std::vector<GLuint> &indices) {
    glm::vec3 c{0.0f, 0.0f, 0.0f};
    float ratio = 1.0f / static_cast<float>(points.size());
    for (size_t i{0}; i < points.size(); ++i) {
        c += points.at(i) * ratio;
    }
    for (size_t i{0}; i < points.size(); ++i) {
        vertices.insert(vertices.end(), {points.at(i).x, points.at(i).y, 0.0f,
                                         0.0f, 0.0f, 1.0f, 0.0f, 1.0f});
        indices.insert(indices.end(),
                       {static_cast<GLuint>(i == 0 ? points.size() - 1 : i - 1),
                        static_cast<GLuint>(i),
                        static_cast<GLuint>(points.size())});
    }
    vertices.insert(vertices.end(),
                    {c.x, c.y, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f});
}
} // namespace

lix::PolygonRendering::PolygonRendering(
    const std::vector<glm::vec3> &points,
    const std::vector<lix::TRS *> &instances)
    : _points{points}, _instances{instances} {
    std::vector<GLfloat> vs;
    std::vector<GLuint> is;
    sortedPolygonToMeshData(points, vs, is);
    _vao = std::make_shared<lix::VAO>(
        lix::Attributes{lix::VEC3, lix::VEC3, lix::VEC2}, vs, is);
}

void lix::PolygonRendering::setPoints(const std::vector<glm::vec3> &points) {
    _points = points;
    std::vector<GLfloat> vs;
    std::vector<GLuint> is;
    sortedPolygonToMeshData(points, vs, is);
    _vao->bind();
    _vao->vbo(0)->bufferData(vs);
    _vao->ebo()->bufferData(is);
}

void lix::PolygonRendering::render(
    std::shared_ptr<lix::ShaderProgram> shaderProgram) {
    for (const auto &inst : _instances) {
        shaderProgram->setUniform("u_model", inst->modelMatrix());
        _vao->bind();
        _vao->draw();
    }
}