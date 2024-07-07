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
#include "physicsengine.h"
#include "gltextrendering.h"
#include "quickhull.h"
#include "convexhull.h"

#include "gen/objects/cube.h"
#include "gen/objects/arrow.h"
#include "gen/objects/chinese_tea_table.h"
#include "gen/ply/bun_zipper_res2.h"

#include "gen/shaders/object_vert.h"
#include "gen/shaders/texture_object_frag.h"
#include "gen/shaders/debug_vao_vert.h"
#include "gen/shaders/debug_vao_frag.h"

#include "gen/fonts/arial.h"

#include "primer.h"
#include "aabb.h"
#include "glcube.h"

//#define SHOW_MINKOWSKI_VOLUME

#define print_var(var) std::cout << #var << "=" << var << std::endl;

inline static constexpr float SCREEN_WIDTH{1080.0f};
inline static constexpr float SCREEN_HEIGHT{720.0f};

std::shared_ptr<lix::VAO> VAOFromConvex(const lix::ConvexHull& ch)
{
    std::vector<GLfloat> mdv;
    std::vector<GLuint> mdi;
    ch.meshData(mdv, mdi);
    return std::shared_ptr<lix::VAO>(new lix::VAO(
        lix::Attributes{lix::VEC3},
        mdv,
        mdi,
        GL_TRIANGLES,
        GL_DYNAMIC_DRAW
    ));
}

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

    void createCube(const lix::Polygon& meshCollider, const lix::Node& node,
        const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, bool dynamic, float density);

    std::shared_ptr<lix::ShaderProgram> shaderProgram;
    std::shared_ptr<lix::ShaderProgram> debugShader;
    std::shared_ptr<lix::Mesh> bunny;
    std::shared_ptr<lix::VAO> convexBunny;
    std::shared_ptr<lix::ConvexHull> bunnyCH;
    //std::shared_ptr<lix::VAO> minkowskiVAO;
    //std::shared_ptr<lix::ConvexHull> convexHull;
    //lix::CollisionSystem collisionSystem;
    std::vector<lix::TextPtr> texts;
    std::unique_ptr<lix::TextRendering> textRendering;
    std::shared_ptr<lix::ConvexHull> simplexHull;
    std::shared_ptr<lix::VAO> simplexHullVAO;
    std::vector<lix::StaticBody> staticBodies;
    std::vector<lix::DynamicBody> dynamicBodies;
    std::vector<std::shared_ptr<lix::Node>> cubeNodes;
    std::vector<std::shared_ptr<lix::Node>> cubeSimpNodes;
    std::vector<std::shared_ptr<lix::Node>> debugRNodes;
};

int main(int argc, const char* argv[])
{
    App app;
    app.run();
    return 0;
}

void computeMinkowski(lix::DynamicBody& bodyA,
    lix::DynamicBody& bodyB,
    std::vector<glm::vec3>& c)
{
    auto polyA = std::dynamic_pointer_cast<lix::Polygon>(bodyA.shape);
    auto polyB = std::dynamic_pointer_cast<lix::Polygon>(bodyB.shape);
    
    if(polyA && polyB)
    {
        const auto& a = polyA->transformedPoints();
        const auto& b = polyB->transformedPoints();

        c.resize(a.size() * b.size());
        for(size_t i{0UL}; i < a.size(); ++i)
        {
            for(size_t j{0UL}; j < b.size(); ++j)
            {
                c[i * a.size() + j] = a[i] - b[j];
            }
        }
    }
}

void App::createCube(const lix::Polygon& meshCollider, const lix::Node& node, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, bool dynamic, float density)
{
    auto cube = lix::NodePtr(node.clone());
    cube->setTranslation(position);
    cube->setRotation(rotation);
    cube->setScale(scale);
    cube->mesh()->material()->setBaseColor(lix::Color::yellow);
    cubeNodes.push_back(cube);

    auto poly = std::shared_ptr<lix::Polygon>(meshCollider.clone());
    poly->setTRS(cube.get());
    auto aabb = std::make_shared<lix::AABB>(cube.get(), poly.get());
    poly->setSimplified(aabb);

    glm::vec3 extents = aabb->max() - aabb->min();
    //glm::vec3 s = extents * scale;
    static bool first{true};
    if(first)
    {
        printf("extens=%.1f %.1f %.1f\n", extents.x, extents.y, extents.z);
        first = false;
    }
    if(dynamic)
    {
        dynamicBodies.push_back(
            lix::PhysicsEngine::createDynamicBody(poly,
                extents.x * extents.y * extents.z * density, extents.x)
        );
    }
    else
    {
        staticBodies.push_back(
            lix::PhysicsEngine::createStaticBody(poly)
        );
    }

    auto cubeSimp = std::make_shared<lix::Node>(position);
    auto pts = aabb->boundingBox();//lix::minimumBoundingBox(aabb->min(), aabb->max());
    
    cubeSimp->setMesh(std::make_shared<lix::Mesh>(
        std::make_shared<lix::VAO>(
        lix::Attributes{lix::Attribute::VEC3},
        (void*)pts.data(), pts.size() * sizeof(glm::vec3),
        (GLuint*)lix::cube_indices.data(), lix::cube_indices.size(),
        GL_TRIANGLES,
        GL_DYNAMIC_DRAW)));

    cubeSimpNodes.push_back(cubeSimp);
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

    bunnyCH = std::make_shared<lix::ConvexHull>(bunnyCloud);
    convexBunny = VAOFromConvex(*bunnyCH);

    camera().setupUBO({
        shaderProgram.get(),
        debugShader.get()
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

    const glm::vec3 origo{0.0f, 0.0f, 0.0f};

    glm::vec3 A{1.0f, 0.0f, 2.0f};
    glm::vec3 B{2.0f, 1.0f, 1.0f};
    glm::vec3 C{3.0f, 0.0f, 2.0f};

    static auto cubeMeshCollider = gltf::loadMeshCollider(assets::objects::chinese_tea_table::Plane_003_mesh, true);
    static auto cubeNode = gltf::loadNode(assets::objects::chinese_tea_table::chinese_tea_table_node);

    static auto bunMeshCollider = gltf::loadMeshCollider(assets::objects::bun_zipper_res2::bun_zipper_res2_mesh, true);
    static auto bunNode = std::make_shared<lix::Node>();
    bunNode->setMesh(gltf::loadMesh(assets::objects::bun_zipper_res2::bun_zipper_res2_mesh));

    createCube(*bunMeshCollider, *bunNode, glm::vec3{-1.0f, 2.0f, 0.5f}, glm::quat{1.0f, 0.0f, 0.0f, 0.0f}, glm::vec3{2.0f}, true, 500.0f);
    /*auto cubeB = createCube(*cubeMeshCollider, *cubeNode, glm::vec3{0.0f, 0.0f, 0.0f}, glm::quat{1.0f, 0.0f, 0.0f, 0.0f}, glm::vec3{1.0f, 1.0f, 1.0f}, false, 500.0f);
    createCube(*cubeMeshCollider, *cubeNode, glm::vec3{0.0f, 0.0f, 3.0f}, glm::quat{1.0f, 0.0f, 0.0f, 0.0f}, glm::vec3{1.0f, 1.0f, 1.0f}, false, 500.0f);*/

    createCube(*cubeMeshCollider, *cubeNode, glm::vec3{-1.0f, -0.5f, 0.0f}, glm::angleAxis(glm::radians(25.0f), glm::vec3{1.0f, 0.0f, 0.0f}), glm::vec3{4.0f, 2.5f, 4.0f} * 0.5f, false, 500.0f);
    createCube(*cubeMeshCollider, *cubeNode, glm::vec3{-1.0f, 0.5f, 3.0f}, glm::angleAxis(glm::radians(60.0f), glm::vec3{-1.0f, 0.0f, 0.0f}), glm::vec3{4.0f, 2.5f, 4.0f} * 0.5f, false, 500.0f);


    static auto createDebugNode = [this](const auto& rb) {
        auto polygon = std::dynamic_pointer_cast<lix::Polygon>(rb.shape);
        if(polygon)
        {
            auto pts = polygon->points();
            std::vector<glm::vec3> rotatedPoints(pts.size());
            std::transform(pts.begin(), pts.end(), rotatedPoints.begin(), [&polygon](const glm::vec3& p) {
                return glm::vec3(glm::scale(polygon->trs()->rotationMatrix(), polygon->trs()->scale()) * glm::vec4{p, 1.0f});
            });
            auto node = std::make_shared<lix::Node>(polygon->trs()->translation());

            auto aabb = dynamic_cast<lix::AABB*>(polygon->simplified());
            std::vector<glm::vec3> minmax = {aabb->min(), aabb->max()};
            auto [vs, is] = lix::cubes_at_points(minmax);

            node->setMesh(std::make_shared<lix::Mesh>(std::make_shared<lix::VAO>(
                lix::Attributes{lix::Attribute::VEC3},
                vs,
                is
            )
            ));
            debugRNodes.push_back(node);
        }
    };

    for(const auto& rb : dynamicBodies)
    {
        createDebugNode(rb);
    }
    for(const auto& rb : staticBodies)
    {
        createDebugNode(rb);
    }

    //std::vector<glm::vec3> minkowski;
    //computeMinkowski(*cubeA, *cubeB, minkowski);

    /*std::vector<glm::vec3> cloud = lix::uniqueVertices(minkowski);
    std::vector<lix::Vertex> minkowskiVertexInfo(cloud.size());
    std::transform(cloud.begin(), cloud.end(), minkowskiVertexInfo.begin(), [](const glm::vec3& v) -> lix::Vertex {
        static uint32_t id{0};
        return {id++, v};
    });
    convexHull = std::make_shared<lix::ConvexHull>(minkowskiVertexInfo);*/

    //minkowskiVAO = VAOFromConvex(*convexHull);

    setSubjectNode(cubeNodes.front());

    std::shared_ptr<lix::Font> font = std::make_shared<lix::Font>(assets::fonts::arial::create());

    auto text = std::make_shared<lix::Text>(font, lix::Text::PropBuilder(), "N/A");
    text->setTranslation(glm::vec3{glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT) * -0.4f, 0.0f});
    texts.insert(texts.end(), {text});
    textRendering.reset(new lix::TextRendering(
        glm::vec2{SCREEN_WIDTH, SCREEN_HEIGHT},
        texts
    ));
}

void App::tick(float dt)
{
    refresh(dt);


    auto aabb_dyn = dynamic_cast<lix::AABB*>(dynamicBodies.front().shape->simplified());
    auto dynSimp = cubeSimpNodes.at(0);
    dynSimp->mesh()->bindVertexArray(0);
    dynSimp->mesh()->vertexArray(0)->vbo(0)->bind();
    auto pts = aabb_dyn->boundingBox();
    dynSimp->mesh()->vertexArray(0)->vbo(0)->bufferData(GL_FLOAT, pts.size() * sizeof(glm::vec3), sizeof(glm::vec3), pts.data());
    dynSimp->setTranslation(aabb_dyn->trs()->translation());

    for(size_t i{0}; i < staticBodies.size(); ++i)
    {
        auto aabb_stat = dynamic_cast<lix::AABB*>(staticBodies.at(i).shape->simplified());
        if(aabb_dyn->intersects(*aabb_stat))
        {
            cubeNodes.at(i + 1)->mesh()->material()->setBaseColor(lix::Color::blue);
        }
        else
        {
            cubeNodes.at(i + 1)->mesh()->material()->setBaseColor(lix::Color::red);
        }
    }

    texts.front()->setText("fps: " + std::to_string(static_cast<int>(fps())));

   lix::PhysicsEngine::step(dynamicBodies, staticBodies, dt);
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
    for(const auto& node : cubeNodes)
    {
        lix::renderNode(*shaderProgram, *node);
    }

    glClear(GL_DEPTH_BUFFER_BIT);
    debugShader->bind();

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    debugShader->setUniform("u_rgb", glm::vec3{1.0f, 0.0f, 1.0f});
    debugShader->setUniform("u_model", glm::mat4{1.0f});
#ifdef SHOW_MINKOWSKI_VOLUME
    minkowskiVAO->bind();
    minkowskiVAO->draw();
#endif
    debugShader->setUniform("u_model", glm::translate(glm::mat4{16.0f}, glm::vec3{0.0f, 0.25f, 0.0f}));
    convexBunny->bind();
    convexBunny->draw();
    if(simplexHullVAO)
    {
        debugShader->setUniform("u_model", glm::mat4{1.0f});
        debugShader->setUniform("u_rgb", glm::vec3{1.0f, 0.0f, 0.0f});
        simplexHullVAO->bind();
        simplexHullVAO->draw();
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    static auto arrowNode = gltf::loadNode(assets::objects::arrow::Arrow_node);
    for(const auto& v : {glm::vec3{1.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 1.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f}})
    {
        debugShader->setUniform("u_rgb", v);
        arrowNode->setRotation(directionToQuat(v));
        lix::renderNode(*debugShader, *arrowNode);
    }

    debugShader->setUniform("u_model", glm::mat4{1.0f});
    debugShader->setUniform("u_rgb", glm::vec3{1.0f, 1.0f, 0.0f});
/*#ifdef SHOW_MINKOWSKI_VOLUME
    glDepthMask(GL_FALSE);
    minkowskiVAO->bind();
    minkowskiVAO->draw();
    glDepthMask(GL_TRUE);
#endif*/

#if 1
    if(simplexHullVAO)
    {
        debugShader->setUniform("u_model", glm::mat4{1.0f});
        debugShader->setUniform("u_rgb", glm::vec3{0.0f, 1.0f, 1.0f});
        simplexHullVAO->bind();
        simplexHullVAO->draw();
    }
#endif

    //debugShader->setUniform("u_model", glm::scale(glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f, 1.0f, 0.0f}), glm::vec3{16.0f}));
    debugShader->setUniform("u_model", glm::translate(glm::mat4{16.0f}, glm::vec3{0.0f, 0.25f, 0.0f}));
    bunny->draw();

    //debugShader->setUniform("u_model", glm::mat4{16.0f});
    //bunny->draw();

    debugShader->setUniform("u_model", glm::mat4{1.0f});

    glCullFace(GL_FRONT);
    debugShader->setUniform("u_rgb", glm::vec3{1.0f, 0.0f, 0.0f});
/*#ifdef SHOW_MINKOWSKI_VOLUME
    minkowskiVAO->bind();
    minkowskiVAO->draw();
#endif*/
    glCullFace(GL_BACK);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //render cubes here
    for(const auto& node : cubeNodes)
    {
        debugShader->setUniform("u_rgb", glm::vec3{1.0f, 1.0f, 0.0f});
        lix::renderNode(*debugShader, *node);
    }

    debugShader->setUniform("u_rgb", glm::vec3{0.0f, 1.0f, 0.0f});
    for(const auto& node : cubeSimpNodes)
    {
        lix::renderNode(*debugShader, *node);
    }

    debugShader->setUniform("u_rgb", glm::vec3{0.0f, 0.0f, 1.0f});
    for(const auto& node : debugRNodes)
    {
        lix::renderNode(*debugShader, *node);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClear(GL_DEPTH_BUFFER_BIT);
    textRendering->render();
}

void App::onSubjectTransformed(std::shared_ptr<lix::Node> /*subject*/, lix::Editor::Transformation /*transformation*/)
{
    /*std::vector<glm::vec3> minkowski;
    computeMinkowski(*rigidBodies.at(0), *rigidBodies.at(1), minkowski);
    minkowskiVAO->bind();
    minkowskiVAO->vbo()->bind();
    minkowskiVAO->vbo()->bufferData(std::vector<GLfloat>{(GLfloat*)minkowski.data(), (GLfloat*)minkowski.data() + minkowski.size() * 3});*/
}