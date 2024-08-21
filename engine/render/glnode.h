#pragma once

#include <functional>
#include <list>
#include <memory>
#include <vector>

#include "glmesh.h"
#include "glskin.h"
#include "gltrs.h"
#include "shape.h"

namespace lix {
class Node;
using NodePtr = std::shared_ptr<Node>;

class Node : public TRS {
  public:
    Node();
    Node(const std::string &name);
    Node(const std::initializer_list<lix::NodePtr> &nodes);
    Node(const glm::vec3 &position,
         const glm::quat &rotation = glm::quat{1.0f, 0.0f, 0.0f, 0.0f},
         const glm::vec3 &scale = glm::vec3{1.0f});

    virtual ~Node() noexcept;

    Node(const TRS &other);

    std::shared_ptr<Node> clone() const;

    Node &operator=(const Node &other) = delete;

    Node(Node &&other) = delete;
    Node &operator=(Node &&other) = delete;

    void appendChild(NodePtr child);

    std::vector<NodePtr> &children();

    const std::vector<NodePtr> &children() const;

    const lix::NodePtr &childAt(size_t index) { return _children.at(index); }

    Node *parent() const;

    void setMesh(const MeshPtr &mesh);

    void setSkin(std::shared_ptr<Skin> skin);

    std::shared_ptr<Skin> skin();

    const std::string &name() const;

    MeshPtr mesh() const;

    Node *find(const std::string &name);

    const glm::mat4 &globalMatrix();

    std::list<Node *> listNodes();

    void forEachChild(const std::function<void(lix::Node &)> &callback);
    void
    forEachChildRecursive(const std::function<void(lix::Node &)> &callback);

    void setVisible(bool visible);
    bool visible() const;

  protected:
    virtual bool updateModelMatrix() override;

    virtual void invalidate() override;

    bool updateGlobalMatrix();

  private:
    Node(const Node &other);

    const std::string &_name;
    std::vector<NodePtr> _children;
    Node *_parent{nullptr};
    MeshPtr _mesh{nullptr};
    glm::mat4 _globalMatrix{1.0f};
    bool _fresch{false}; // is global matrix fresch?
    std::shared_ptr<Skin> _skin{nullptr};
    std::shared_ptr<lix::Shape> _shape;
    bool _visible{true};
};
} // namespace lix