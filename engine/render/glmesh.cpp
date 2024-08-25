#include "glmesh.h"

const lix::Material lix::Mesh::defaultMaterial{{1.0f, 1.0f, 1.0f, 1.0f}};

inline static const std::string noName{""};

lix::Mesh::Mesh() : _name{noName}, _primitives{} {}

lix::Mesh::Mesh(const std::string &name) : _name{name}, _primitives{} {}

lix::Mesh::Mesh(std::shared_ptr<lix::VertexArray> vao,
                std::shared_ptr<lix::Material> material)
    : Mesh(noName) {
    _primitives.emplace_back(
        vao,
        material ? material : std::make_shared<lix::Material>(defaultMaterial));
}

lix::Mesh::Mesh(const std::string &name, std::shared_ptr<lix::VertexArray> vao,
                std::shared_ptr<lix::Material> material)
    : Mesh{name} {
    _primitives.emplace_back(
        vao,
        material ? material : std::make_shared<lix::Material>(defaultMaterial));
}

lix::Mesh::Mesh(const Mesh &other) : _name{other._name} {
    for (size_t i{0}; i < other._primitives.size(); ++i) {
        const auto &op = other._primitives.at(i);
        _primitives.emplace_back(
            std::make_shared<lix::VertexArray>(*op.vao),
            std::make_shared<lix::Material>(op.material ? *op.material
                                                        : defaultMaterial));
    }
}

lix::Mesh::~Mesh() noexcept {}

lix::Mesh *lix::Mesh::clone() const { return new Mesh(*this); }

const lix::Mesh::Primitive &
lix::Mesh::createPrimitive(GLenum mode,
                           std::shared_ptr<lix::Material> material) {
    return _primitives.emplace_back(
        std::make_shared<lix::VertexArray>(mode),
        material ? material : std::make_shared<lix::Material>(defaultMaterial));
}

const lix::Mesh::Primitive &lix::Mesh::primitive(size_t index) const {
    return _primitives.at(index);
}

std::shared_ptr<lix::Material> lix::Mesh::material(size_t index) const {
    return index < _primitives.size() ? _primitives.at(index).material
                                      : nullptr;
}

std::shared_ptr<lix::VertexArray> lix::Mesh::vertexArray(size_t index) const {
    return index < _primitives.size() ? _primitives.at(index).vao : nullptr;
}

size_t lix::Mesh::count() const { return _primitives.size(); }

std::shared_ptr<lix::VertexArray> lix::Mesh::bindVertexArray(size_t index) {
    const auto &prim = _primitives.at(index);
    prim.vao->bind();
    return prim.vao;
}

void lix::Mesh::draw(size_t index) {
    const auto &prim = _primitives.at(index);
    prim.vao->bind();
    prim.vao->draw();
}

void lix::Mesh::draw() {
    for (size_t i{0}; i < _primitives.size(); ++i) {
        draw(i);
    }
}