#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "glm/gtc/random.hpp"

#include "glapplication.h"
#include "glcamera.h"
#include "glrendering.h"
#include "glshaderprogram.h"
#include "gleditor.h"
#include "polygon.h"
#include "gltfloader.h"
#include "collision.h"
#include "gltextrendering.h"
#include "quickhull.h"

#include "convexhull.h"

#include "primer.h"

#include "gen/objects/cube.h"
#include "gen/objects/arrow.h"
#include "gen/ply/bun_zipper_res2.h"

#include "gen/shaders/object_vert.h"
#include "gen/shaders/texture_object_frag.h"
#include "gen/shaders/debug_vao_vert.h"
#include "gen/shaders/debug_vao_frag.h"

#include "gen/fonts/arial.h"

#define print_var(var) std::cout << #var << "=" << var << std::endl;

inline static constexpr float SCREEN_WIDTH{600.0f};
inline static constexpr float SCREEN_HEIGHT{600.0f};

struct RigidBody
{
    unsigned int id;
    std::shared_ptr<lix::Node> node;
    lix::Polygon polygon;
    lix::VertexArray debugVao;
    bool collides;
    bool dynamic;
    glm::vec3 velocity;
};

struct CollisionTrack
{
    glm::vec3 positionA;
    glm::vec3 positionB;
    std::shared_ptr<RigidBody> bodyA;
    std::shared_ptr<RigidBody> bodyB;
    lix::Collision collision;
    std::vector<glm::vec3> simplex;
};

glm::quat directionToQuat(const glm::vec3& direction)
{
    glm::vec3 n = glm::normalize(direction);
    glm::vec3 UP{0.0f, 1.0f, 0.0f};
    float angle;
    glm::vec3 axis;
    
    if(n.y >= 1.0f)
    {
        angle = 0.0f;
        axis = glm::vec3{1.0f, 0.0f, 0.0f};
    }
    else if(n.y <= -1.0f)
    {
        angle = glm::pi<float>();
        axis = glm::vec3{1.0f, 0.0f, 0.0f};
    }
    else
    {
        axis = glm::normalize(glm::cross(UP, n));
        angle = acosf(glm::dot(UP, n));
    }
    
    return glm::angleAxis(angle, axis);
}

glm::vec3 quatToDirection(const glm::quat& quat)
{
    glm::vec3 UP{0.0f, 1.0f, 0.0f};
    return quat * UP;
}

struct Arrow
{
    Arrow(std::shared_ptr<lix::Node> node_, const glm::vec3& position, const glm::vec3& direction, const lix::Color& color_)
        : node{node_}, color{color_}
    {
        setPosition(position);
        setDirection(direction);
    }

    void setPosition(const glm::vec3& position)
    {
        node->setTranslation(position);
    }

    void setDirection(const glm::vec3& direction)
    {
        node->setRotation(directionToQuat(direction));
    }

    void set(const glm::vec3& position, const glm::vec3& direction, const lix::Color& color_)
    {
        setPosition(position);
        setDirection(direction);
        color = color_;
    }

    const glm::vec3& position() const
    {
        return node->translation();
    }

    glm::vec3 direction()
    {
        return quatToDirection(node->rotation());
    }

    lix::NodePtr node;
    lix::Color color;
};

struct App : public lix::Application, public lix::Editor
{
    App() : lix::Application{static_cast<int>(SCREEN_WIDTH), static_cast<int>(SCREEN_HEIGHT), "test_collision"},
        lix::Editor{glm::perspective(glm::radians(45.0f), SCREEN_WIDTH / SCREEN_HEIGHT, 0.01f, 100.0f),
            glm::vec2{SCREEN_WIDTH, SCREEN_HEIGHT}
        }
    {

    }

    virtual void init() override;

    virtual void tick(float dt) override;

    virtual void draw() override;

    virtual void onSubjectTransformed(std::shared_ptr<lix::Node> subject, lix::Editor::Transformation transformation) override;

    void setArrowsTriangle(const std::vector<glm::vec3>& simplex);
    void setArrowsTetrahedron(const std::vector<glm::vec3>& simplex);

    void setArrowsHalfEdge();

    void printCurEdge();

    std::vector<std::shared_ptr<RigidBody>> rigidBodies;
    std::shared_ptr<lix::ShaderProgram> shaderProgram;
    std::shared_ptr<lix::ShaderProgram> debugShader;
    std::vector<Arrow> arrows;
    std::shared_ptr<lix::Mesh> bunny;
    std::shared_ptr<lix::VAO> convexBunny;
    std::shared_ptr<lix::Convex_Hull> bunnyCH;
    std::shared_ptr<lix::VAO> minkowskiVAO;
    bool shouldSetArrows{true};
    std::unordered_map<unsigned int, std::unordered_map<unsigned int, lix::Collision>> collisions;
    std::vector<CollisionTrack> collisionTracks;
    std::vector<CollisionTrack>::iterator collisionTracksIt;
    unsigned int collisionTrackIdentity;
    bool traceCollisions{false};

    lix::Half_Edge* curEdge{nullptr};
    std::shared_ptr<lix::Convex_Hull> convexHull;
    std::list<lix::Face>::iterator curFaceIt;

    std::vector<lix::TextPtr> texts;
    std::unique_ptr<lix::TextRendering> textRendering;
};

int main(int argc, const char* argv[])
{
    App app;
    app.run();
    return 0;
}

void App::printCurEdge()
{
    std::cout << "edge: <" << curEdge->id << "> f" <<  curEdge->face->id << " opposite: " << (curEdge->opposite ? std::to_string(curEdge->opposite->face->id) : "null") << std::endl;
}

void computeMinkowski(RigidBody& bodyA,
    RigidBody& bodyB,
    std::vector<glm::vec3>& c)
{
    auto& a = bodyA.polygon.transformedPoints();
    auto& b = bodyB.polygon.transformedPoints();

    c.resize(a.size() * b.size());
    for(size_t i{0UL}; i < a.size(); ++i)
    {
        for(size_t j{0UL}; j < b.size(); ++j)
        {
            c[i * a.size() + j] = a[i] - b[j];
        }
    }
}

void createCube(std::vector<std::shared_ptr<RigidBody>>& rigidBodies, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
{
    static auto vertexPositions = gltf::loadVertexPositions(assets::objects::cube::Cube_mesh);
    static std::vector<glm::vec3> cube_vertices;
    static std::vector<GLushort> cube_indices;
    static std::vector<glm::vec3> cube_unique;
    static bool first{true};
    if(first)
    {
        gltf::loadAttributes(assets::objects::cube::Cube_mesh, 0, gltf::A_POSITION, cube_vertices, cube_indices);
        lix::uniqueVertices(cube_vertices, cube_unique);
        
        print_var(cube_vertices.size());
        print_var(cube_unique.size());
        first = false;
    }

    auto cube = gltf::loadNode(assets::objects::cube::Cube_node);
    cube->setTranslation(position);
    cube->setRotation(rotation);
    cube->setScale(scale);
    cube->mesh()->material()->setBaseColor(lix::Color::yellow);
    rigidBodies.push_back(std::shared_ptr<RigidBody>(
        new RigidBody{static_cast<unsigned int>(rigidBodies.size()), cube, {cube_unique}, {
            lix::Attributes{lix::VEC3},
            std::vector<GLfloat>{(GLfloat*)vertexPositions.data(), (GLfloat*)vertexPositions.data() + vertexPositions.size() * 3},
            GL_TRIANGLES,
            GL_STATIC_DRAW},
            false,
            false,
            {0.0f, 0.0f, 0.0f}
        }
    ));
    rigidBodies.back()->polygon.setPosition(position);
    rigidBodies.back()->polygon.setRotation(rotation);
    rigidBodies.back()->polygon.setScale(scale);
}

void App::init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    //glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    setInputAdapter(this);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    shaderProgram.reset(new lix::ShaderProgram(
        assets::shaders::object_vert,
        assets::shaders::texture_object_frag
    ));
    debugShader.reset(new lix::ShaderProgram(
        assets::shaders::debug_vao_vert,
        assets::shaders::debug_vao_frag
    ));

    bunny = gltf::loadMesh(assets::objects::bun_zipper_res2::bun_zipper_res2_mesh);

    const gltf::Buffer* bunnyPosAttrib = assets::objects::bun_zipper_res2::bun_zipper_res2_mesh.primitives[0].attributes[0];

    std::vector<glm::vec3> bunnyCloud{
        (glm::vec3*)bunnyPosAttrib->data,
        (glm::vec3*)bunnyPosAttrib->data + bunnyPosAttrib->data_size / sizeof(glm::vec3)};

    bunnyCH = std::make_shared<lix::Convex_Hull>(bunnyCloud);
    {
        std::vector<GLfloat> mdv;
        std::vector<GLuint> mdi;
        bunnyCH->mesh_data(mdv, mdi);
        printf("#mdv=%zu #mdi=%zu\n", mdv.size(), mdi.size());

        convexBunny.reset(new lix::VAO(
            lix::Attributes{lix::VEC3},
            //std::vector<GLfloat>{(GLfloat*)convex_vertices.data(), (GLfloat*)convex_vertices.data() + convex_vertices.size() * 3},
            //convex_indices,
            mdv,
            mdi,
            GL_TRIANGLES,
            GL_DYNAMIC_DRAW
        ));
    }

    camera().setupUBO({
        shaderProgram.get(),
        debugShader.get()
    });

    setOnKeyDown(SDLK_j, [this](auto key, auto mod) {
        if(!traceCollisions)
        {
            collisionTracksIt = collisionTracks.end() - 1;
            traceCollisions = true;
        }
        else
        {
            if(collisionTracksIt == collisionTracks.begin())
            {
                collisionTracksIt = collisionTracks.end() - 1;
            }
            else
            {
                --collisionTracksIt;
            }
        }
    });

    setOnKeyDown(SDLK_n, [this](auto key, auto mod) {
        if(mod & KMOD_ALT)
        {
            curEdge = curEdge->next;
            printCurEdge();
            setArrowsHalfEdge();
        }
        else if(traceCollisions)
        {
            if(collisionTracksIt == collisionTracks.end() - 1)
            {
                collisionTracksIt = collisionTracks.begin();
            }
            else
            {
                ++collisionTracksIt;
            }
        }
    });

    setOnKeyDown(SDLK_f, [this](auto, auto mod) {
        if(mod & KMOD_ALT)
        {
            ++curFaceIt;
            if(curFaceIt == convexHull->end())
            {
                curFaceIt = convexHull->begin();
            }
            curEdge = curFaceIt->half_edge;
            printCurEdge();
            setArrowsHalfEdge();
        }
    });

    setOnKeyDown(SDLK_o, [this](auto, auto mod) {
        if(mod & KMOD_ALT)
        {
            curEdge = curEdge->opposite;
            printCurEdge();
            setArrowsHalfEdge();
        }
    });

    setOnKeyDown(SDLK_c, [this](auto key, auto mod) {
        static bool cullFace{true};
        if(cullFace)
        {
            glDisable(GL_CULL_FACE);
            cullFace = false;
        }
        else
        {
            glEnable(GL_CULL_FACE);
            cullFace = true;
        }
    });

    /*for(const auto& v : vertexPositions)
    {
        std::cout << "[" << v.x << ", " << v.y << ", " << v.z << "]" << std::endl;
    }*/

    auto arrowNode = gltf::loadNode(assets::objects::arrow::Arrow_node);
    const glm::vec3 origo{0.0f, 0.0f, 0.0f};


    glm::vec3 A{1.0f, 0.0f, 2.0f};
    glm::vec3 B{2.0f, 1.0f, 1.0f};
    glm::vec3 C{3.0f, 0.0f, 2.0f};

    std::vector<glm::vec3> test_s = {A, B, C};

    for(size_t i{0}; i < 64; ++i)
    {
        arrows.push_back({arrowNode->clone(), glm::vec3{9999.0f}, glm::vec3{1.0f, 0.0f, 0.0f}, lix::Color::red});
    }
    setArrowsTriangle(test_s);

    createCube(rigidBodies, glm::vec3{0.0f, 0.0f, 0.0f}, glm::quat{1.0f, 0.0f, 0.0f, 0.0f}, glm::vec3{1.0f, 1.0f, 1.0f});
    createCube(rigidBodies, glm::vec3{0.0f, 0.0f, 3.0f}, glm::quat{1.0f, 0.0f, 0.0f, 0.0f}, glm::vec3{1.0f, 1.0f, 1.0f});

    createCube(rigidBodies, glm::vec3{-4.0f, -1.0f, 0.0f}, glm::angleAxis(glm::radians(15.0f), glm::vec3{1.0f, 0.0f, 0.0f}), glm::vec3{2.0f, 0.2f, 2.0f});
    createCube(rigidBodies, glm::vec3{-4.0f, 2.0f, 0.0f}, glm::quat{1.0f, 0.0f, 0.0f, 0.0f}, glm::vec3{0.2f, 0.2f, 0.2f});
    collisionTrackIdentity = rigidBodies.back()->id;
    rigidBodies.back()->dynamic = true;
    createCube(rigidBodies, glm::vec3{-4.0f, 1.0f, 3.0f}, glm::angleAxis(glm::radians(-60.0f), glm::vec3{1.0f, 0.0f, 0.0f}), glm::vec3{2.0f, 0.2f, 2.0f});

    auto& cubeA = rigidBodies.at(0);
    auto& cubeB = rigidBodies.at(1);

    std::vector<glm::vec3> minkowski;
    computeMinkowski(*cubeA, *cubeB, minkowski);

    std::vector<glm::vec3> cloud;
    lix::uniqueVertices(minkowski, cloud);
    convexHull = std::make_shared<lix::Convex_Hull>(cloud);
    std::vector<GLfloat> mdv;
    std::vector<GLuint> mdi;
    convexHull->mesh_data(mdv, mdi);
    printf("#mdv=%zu #mdi=%zu\n", mdv.size(), mdi.size());
    curEdge = convexHull->begin()->half_edge;
    curFaceIt = convexHull->begin();
    setArrowsHalfEdge();

    minkowskiVAO.reset(new lix::VAO(
            lix::Attributes{lix::VEC3},
            //std::vector<GLfloat>{(GLfloat*)convex_vertices.data(), (GLfloat*)convex_vertices.data() + convex_vertices.size() * 3},
            //convex_indices,
            mdv,
            mdi,
            GL_TRIANGLES,
            GL_DYNAMIC_DRAW
    ));

    setSubjectNode(rigidBodies.front()->node);

    std::shared_ptr<lix::Font> font = std::make_shared<lix::Font>(assets::fonts::arial::create());

    auto text = std::make_shared<lix::Text>(font, lix::Text::PropBuilder(), "N/A");
    text->setTranslation(glm::vec3{glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT) * -0.4f, 0.0f});
    texts.insert(texts.end(), {text});
    textRendering.reset(new lix::TextRendering(
        glm::vec2{SCREEN_WIDTH, SCREEN_HEIGHT},
        texts
    ));
}

void App::setArrowsTriangle(const std::vector<glm::vec3>& simplex)
{
#if 1
    const glm::vec3 origo{0.0f, 0.0f, 0.0f};
    const glm::vec3& C = simplex.at(0);
    const glm::vec3& B = simplex.at(1);
    const glm::vec3& A = simplex.at(2);

    glm::vec3 DirToA = glm::cross(glm::cross(C - B, -B), C - B);
    
    glm::vec3 AB = B - A;
    glm::vec3 AC = C - A;    

    glm::vec3 ABC_normal = glm::cross(AC, AB);
    glm::vec3 ABC_center = (A + B + C) / 3.0f;

    glm::vec3 AB_center = (A + B) / 2.0f;
    glm::vec3 AB_normal = glm::cross(ABC_normal, AB);

    glm::vec3 AC_center = (A + C) / 2.0f;
    glm::vec3 AC_normal = glm::cross(AC, ABC_normal);

    arrows.at(0).set(origo, glm::vec3{1.0f, 0.0f, 0.0f}, lix::Color::red);
    arrows.at(1).set(origo, glm::vec3{0.0f, 1.0f, 0.0f}, lix::Color::green);
    arrows.at(2).set(origo, glm::vec3{0.0f, 0.0f, 1.0f}, lix::Color::blue);
    arrows.at(3).set(A, ABC_normal, lix::Color::red);
    arrows.at(4).set(B, ABC_normal, lix::Color::green);
    arrows.at(5).set(C, ABC_normal, lix::Color::blue);
    arrows.at(6).set(A, AB, lix::Color::magenta);
    arrows.at(7).set(A, AC, lix::Color::magenta);
    arrows.at(8).set(ABC_center, ABC_normal, lix::Color::magenta);
    arrows.at(9).set(AB_center, AB_normal, lix::Color::magenta);
    arrows.at(10).set(AC_center, AC_normal, lix::Color::magenta);
    arrows.at(11).set((C + B) / 2.0f, DirToA, lix::Color::cyan);

    glm::vec3 BC = C - B;
    float t = -(glm::dot(BC, B) / glm::dot(BC, BC));
    glm::vec3 rt = B + BC * t;

    arrows.at(12).set(rt, glm::normalize(-rt), lix::Color::yellow);
#endif
}

void App::setArrowsTetrahedron(const std::vector<glm::vec3>& simplex)
{
#if 1
    setArrowsTriangle(simplex);

    const glm::vec3 origo{0.0f, 0.0f, 0.0f};
    const glm::vec3& D = simplex.at(0);
    const glm::vec3& C = simplex.at(1);
    const glm::vec3& B = simplex.at(2);
    const glm::vec3& A = simplex.at(3);

    const glm::vec3 BC = C - B;
    const glm::vec3 BD = D - B;
    const glm::vec3 BCD = glm::cross(BC, BD);

    arrows.at(20).set(A, BCD, lix::Color::white);
#endif
}

void App::setArrowsHalfEdge()
{
    glm::vec3 verts[3];
    lix::Half_Edge* e = curEdge;
    for(size_t i{0}; i < 3; ++i)
    {
        verts[i] = e->vertex;
        e = e->next;
    }

    const glm::vec3& a = verts[0];
    const glm::vec3& b = verts[1];
    const glm::vec3& c = verts[2];
    const glm::vec3 ab = b - a;
    const glm::vec3 bc = c - b;
    const glm::vec3 ca = a - c;
    const glm::vec3 ac = c - a;
    const glm::vec3 abc = glm::normalize(glm::cross(ab, ac));

    arrows.at(24).set(a, ab, 0xff0000);
    arrows.at(25).set(b, bc, 0xff8800);
    arrows.at(26).set(c, ca, 0xff8800);
}

void collisionResponse(RigidBody& rigidBody, glm::vec3 normal, float penetration)
{
    rigidBody.velocity = glm::reflect(rigidBody.velocity, normal);
    rigidBody.node->applyTranslation(normal * penetration);
}

glm::vec3 contactPoint(lix::Polygon& polygonA,lix::Polygon& polygonB, const glm::vec3& collisionNormal)
{
    auto s = polygonA.storedSimplex();

    if(s.size() > 2)
    {
        return (s[0] + s[1] + s[2]) / 3.0f;
    }
    else if(s.size() > 1)
    {
        return (s[0] + s[1]) / 2.0f;
    }
    else if(s.size() > 0)
    {
        return s[0];
    }
    return polygonA.supportPoint(collisionNormal);
}

void App::tick(float dt)
{
    refresh(dt);

    if(traceCollisions)
    {
        static auto cacheIt = collisionTracks.end();
        if(collisionTracksIt != cacheIt)
        {
            switch(collisionTracksIt->simplex.size())
            {
            case 3:
                setArrowsTriangle(collisionTracksIt->simplex);
                break;
            case 4:
                setArrowsTetrahedron(collisionTracksIt->simplex);
                break;
            }
            
            collisionTracksIt->bodyA->polygon.setPosition(collisionTracksIt->positionA);
            collisionTracksIt->bodyB->polygon.setPosition(collisionTracksIt->positionB);
            auto contactA = contactPoint(collisionTracksIt->bodyA->polygon, collisionTracksIt->bodyA->polygon, collisionTracksIt->collision.collisionNormal);
            arrows.at(30).setPosition(contactA);
            arrows.at(30).setDirection(collisionTracksIt->collision.collisionNormal);

            texts.at(0)->setText(std::to_string(collisionTracksIt->collision.penetrationDepth));
            cacheIt = collisionTracksIt;

            collisionTracksIt->bodyA->polygon.setPosition(collisionTracksIt->positionA);
            collisionTracksIt->bodyA->node->setTranslation(collisionTracksIt->positionA);
            collisionTracksIt->bodyB->polygon.setPosition(collisionTracksIt->positionB);
            collisionTracksIt->bodyB->node->setTranslation(collisionTracksIt->positionB);

            std::vector<glm::vec3> minkowski;
            computeMinkowski(*collisionTracksIt->bodyA, *collisionTracksIt->bodyB, minkowski);

            std::vector<glm::vec3> cloud;
            lix::uniqueVertices(minkowski, cloud);
            convexHull = std::make_shared<lix::Convex_Hull>(cloud);
            std::vector<GLfloat> mdv;
            std::vector<GLuint> mdi;
            convexHull->mesh_data(mdv, mdi);
            printf("#mdv=%zu #mdi=%zu\n", mdv.size(), mdi.size());
            curEdge = convexHull->begin()->half_edge;
            curFaceIt = convexHull->begin();
            setArrowsHalfEdge();

            minkowskiVAO->bind();
            minkowskiVAO->vbo()->bind();
            minkowskiVAO->vbo()->bufferData(mdv);
            minkowskiVAO->ebo()->bind();
            minkowskiVAO->ebo()->bufferData(mdi);
        }
        return;
    }

    for(const auto& cr1 : collisions)
    {
        for(const auto& cr2 : cr1.second)
        {
            std::shared_ptr<RigidBody> r1 =  rigidBodies.at(cr1.first);
            std::shared_ptr<RigidBody> r2 =  rigidBodies.at(cr2.first);
            printf("resolve collision: %d, %d\n", r1->id, r2->id);
            if(r1->dynamic)
            {
                collisionResponse(*r1, cr2.second.collisionNormal, r2->dynamic
                    ? (cr2.second.penetrationDepth * 0.5f)
                    : cr2.second.penetrationDepth);
            }
            if(r2->dynamic)
            {
                collisionResponse(*r2, -cr2.second.collisionNormal, r1->dynamic
                    ? (cr2.second.penetrationDepth * 0.5f)
                    : cr2.second.penetrationDepth);
            }
        }
    }
    collisions.clear();

    for(const auto& rigidBody : rigidBodies)
    {
        if(rigidBody->dynamic)
        {
            rigidBody->velocity.y = std::max(-8.0f, rigidBody->velocity.y - 2.0f * dt);
            rigidBody->node->applyTranslation(rigidBody->velocity * dt);
        }
        rigidBody->polygon.setPosition(rigidBody->node->translation());
        rigidBody->collides = false;
    }

    for(const auto& bodyA : rigidBodies)
    {
        for(const auto& bodyB : rigidBodies)
        {
            if(bodyA->id >= bodyB->id)
            {
                continue;
            }
            unsigned int minId = bodyA->id;
            unsigned int maxId = bodyB->id;
            auto minIt = collisions.find(minId);
            if(minIt != collisions.end())
            {
                auto maxIt = minIt->second.find(maxId);
                if(maxIt != minIt->second.end())
                {
                    std::cout << "already colliding!" << std::endl;
                    continue; // already colliding
                }
            }
            std::vector<glm::vec3> simplex;
            glm::vec3 D = glm::ballRand(1.0f);
            lix::Collision collision;
            if(lix::gjk(bodyA->polygon, bodyB->polygon, simplex, D, &collision))
            {
                //assert(simplex.size() == 4);
                const glm::vec3& n = collision.collisionNormal;
                switch(simplex.size())
                {
                    case 2:
                        std::cout << "EDGE";
                        break;
                    case 4:
                        std::cout << "POLYHEDRA";
                        break;
                    default:
                        std::cout << "???";
                        break;
                }
                if(minId == collisionTrackIdentity || maxId == collisionTrackIdentity)
                {
                    std::cout << " " << n.x << ", " << n.y << ", " << n.z
                        << " : " << collision.penetrationDepth
                        << std::endl;
                    auto body = bodyA->id == collisionTrackIdentity ? bodyA : bodyB;
                    auto other = bodyA->id == collisionTrackIdentity ? bodyB : bodyA;
                    collisionTracks.push_back({body->polygon.position(), other->polygon.position(), body, other, collision, {simplex.begin(), simplex.end()}});
                    texts.at(0)->setText(std::to_string(collision.penetrationDepth));
                }
                if(bodyA->dynamic || bodyB->dynamic)
                {
                    collisions[minId][maxId] = std::move(collision);
                }
                bodyA->collides = true;
                bodyB->collides = true;
            }
            if(shouldSetArrows)
            {
                switch(simplex.size())
                {
                case 3:
                    setArrowsTriangle(simplex);
                    break;
                case 4:
                    setArrowsTetrahedron(simplex);
                    break;
                }
                shouldSetArrows = false;
            }
        }
    }

}

void App::draw()
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    
    shaderProgram->bind();
    static auto basicTex = lix::Texture::Basic(lix::Color::white);
    basicTex->bind();
    /*for(const auto& rigidBody : rigidBodies)
    {
        lix::renderNode(*shaderProgram, *rigidBody->node);
    }*/

    glClear(GL_DEPTH_BUFFER_BIT);
    debugShader->bind();

    for(const auto& arrow : arrows)
    {
        debugShader->setUniform("u_rgb", glm::vec3(arrow.color.vec4()));
        lix::renderNode(*debugShader, *arrow.node);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    debugShader->setUniform("u_rgb", glm::vec3{1.0f, 0.0f, 1.0f});
    debugShader->setUniform("u_model", glm::mat4{1.0f});
    minkowskiVAO->bind();
    minkowskiVAO->draw();
    debugShader->setUniform("u_model", glm::mat4{16.0f});
    convexBunny->bind();
    convexBunny->draw();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    debugShader->setUniform("u_rgb", glm::vec3{1.0f, 1.0f, 0.0f});
    debugShader->setUniform("u_model", glm::mat4{1.0f});
    minkowskiVAO->bind();
    minkowskiVAO->draw();

    //debugShader->setUniform("u_model", glm::scale(glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f, 1.0f, 0.0f}), glm::vec3{16.0f}));
    debugShader->setUniform("u_model", glm::mat4{16.0f});
    bunny->draw();

    //debugShader->setUniform("u_model", glm::mat4{16.0f});
    //bunny->draw();

    debugShader->setUniform("u_model", glm::mat4{1.0f});

    glCullFace(GL_FRONT);
    debugShader->setUniform("u_rgb", glm::vec3{1.0f, 0.0f, 0.0f});
    minkowskiVAO->bind();
    minkowskiVAO->draw();
    glCullFace(GL_BACK);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    for(const auto& rigidBody : rigidBodies)
    {
        debugShader->setUniform("u_model", rigidBody->node->model());
        debugShader->setUniform("u_rgb", glm::vec3(rigidBody->collides
            ? lix::Color::red.vec4() : lix::Color::green.vec4()));
        rigidBody->debugVao.bind();
        rigidBody->debugVao.draw();
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClear(GL_DEPTH_BUFFER_BIT);
    textRendering->render();
}

void App::onSubjectTransformed(std::shared_ptr<lix::Node> /*subject*/, lix::Editor::Transformation /*transformation*/)
{
    std::vector<glm::vec3> minkowski;
    computeMinkowski(*rigidBodies.at(0), *rigidBodies.at(1), minkowski);
    minkowskiVAO->bind();
    minkowskiVAO->vbo()->bind();
    minkowskiVAO->vbo()->bufferData(std::vector<GLfloat>{(GLfloat*)minkowski.data(), (GLfloat*)minkowski.data() + minkowski.size() * 3});
    shouldSetArrows = true;
}