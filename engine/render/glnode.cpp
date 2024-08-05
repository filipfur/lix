#include "glnode.h"


lix::Node::Node() : TRS()
{
    
}

lix::Node::Node(const std::initializer_list<lix::NodePtr>& children) : TRS()
{
    for(const auto& child : children)
    {
        appendChild(child);
    }
}

lix::Node::Node(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
    : TRS{position, rotation, scale}
{

}

lix::Node::~Node() noexcept
{
    _children.clear();
    _parent = nullptr;
}

lix::Node::Node(const lix::TRS& other) : TRS{other}
{

}

lix::Node::Node(const lix::Node& other)
    : TRS{other}, _name{other._name}, _mesh{other._mesh->clone()}, _skin{other._skin}
{
    
}

std::shared_ptr<lix::Node> lix::Node::clone() const
{
    auto node = std::shared_ptr<lix::Node>(
        new lix::Node(*this)
    );
    for(auto& child : _children)
    {
        auto c = child->clone();
        node->_children.push_back(c);
        c->_parent = node.get();
    }
    return node;
}

void lix::Node::appendChild(NodePtr child)
{
    _children.emplace_back(child);
    child->_parent = this;
}

std::vector<lix::NodePtr>& lix::Node::children() { return _children; }

const std::vector<lix::NodePtr>& lix::Node::children() const { return _children; }

lix::Node* lix::Node::parent() const { return _parent; }

void lix::Node::setMesh(MeshPtr mesh) { _mesh = mesh; }

void lix::Node::setSkin(std::shared_ptr<Skin> skin)
{
    _skin = skin;
}

std::shared_ptr<lix::Skin> lix::Node::skin() 
{
    return _skin;
}

void lix::Node::setName(const std::string& name) { _name = name; }

const std::string& lix::Node::name() const { return _name; }

lix::MeshPtr lix::Node::mesh() const { return _mesh; }

lix::Node* lix::Node::find(const std::string& name)
{
    if(name == _name)
    {
        return this;
    }
    for(const auto& child : _children)
    {
        Node* n = child->find(name);
        if(n)
        {
            return n;
        }
    }
    return nullptr;
}

const glm::mat4& lix::Node::globalMatrix()
{
    updateGlobalMatrix();
    return _globalMatrix;
}

std::list<lix::Node*> lix::Node::listNodes()
{
    std::list<Node*> nodes;
    nodes.push_back(this);
    forEachChildRecursive([&nodes](lix::Node& node){
        nodes.push_back(&node);
    });
    return nodes;
}

void _forEachChild(lix::Node& node, const std::function<void(lix::Node&)>& callback, bool recursive=false)
{
    for(const auto& child : node.children())
    {
        callback(*child);
        if(recursive)
        {
            _forEachChild(*child, callback, recursive);
        }
    }
}

void lix::Node::forEachChild(const std::function<void(lix::Node&)>& callback)
{
    _forEachChild(*this, callback, false);
}

void lix::Node::forEachChildRecursive(const std::function<void(lix::Node&)>& callback)
{
    _forEachChild(*this, callback, true);
}

void lix::Node::setVisible(bool visible)
{
    _visible = visible;
}

bool lix::Node::visible() const
{
    return _visible;
}

bool lix::Node::updateModelMatrix()
{
    if(TRS::updateModelMatrix())
    {
        _fresch = false;
        forEachChildRecursive([](lix::Node& child) {
            child._fresch = false;
        });
        return true;
    }
    return false;
}

void lix::Node::invalidate()
{
    TRS::invalidate();
    _fresch = false;
}

bool lix::Node::updateGlobalMatrix()
{
    if(_fresch)
    {
        return false;
    }
    if(_parent)
    {
        _parent->updateGlobalMatrix();
        _globalMatrix = _parent->globalMatrix() * modelMatrix();
    }
    else
    {
        _globalMatrix = modelMatrix();
    }
    _fresch = true;
    return true;
}