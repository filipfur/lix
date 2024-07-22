#include <fstream>
#include "glapplication.h"
#include "gleditor.h"

#include "glshaderprogram.h"
#include "gluniformbuffer.h"
#include "gltfloader.h"
#include "glrendering.h"
#include "json.h"

#include "rigidbody.h"
#include "collision.h"
#include "primer.h"
#include "aabb.h"
#include "sphere.h"
#include "polygon.h"
#include "glcube.h"
#include "inertia.h"
#include "charactercontroller.h"
#include "convexhull.h"

#include "gen/objects/cube.h"
#include "gen/objects/platform.h"
#include "gen/objects/spyro_ps1.h"
#include "gen/objects/macman.h"
#include "gen/objects/donut.h"

#include "gen/levels/level.h"

static constexpr float WINDOW_X = 1920;
static constexpr float WINDOW_Y = 1080;
static constexpr float NEAR = 0.01;
static constexpr float FAR = 100;
static constexpr float CAMERA_DISTANCE = 12.0f;
static const glm::mat4 PERSPECTIVE{glm::perspective(45.0f, WINDOW_X / WINDOW_Y, NEAR, FAR)};
static const glm::vec2 RESOLUTION{WINDOW_X, WINDOW_Y};

const char* vertexSource = LIX_SHADER_VERSION R"(
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexCoords;

    layout (std140) uniform CameraBlock
    {
        mat4 u_projection;
        mat4 u_view;
        vec3 u_eye_pos;
    };
    uniform mat4 u_model;

    out vec3 position;
    out vec3 normal;
    out vec2 texCoords;

    void main()
    {
        position = vec3(u_model * vec4(aPos, 1.0));
        normal = normalize(mat3(u_model) * aNormal);
        texCoords = aTexCoords;
        gl_Position = u_projection * u_view * vec4(position, 1.0);
    }
)";

const char* vertexSkinSource = LIX_SHADER_VERSION R"(
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexCoords;
    layout (location=3) in uvec4 aJoints;
    layout (location=4) in vec4 aWeights;

    layout (std140) uniform CameraBlock
    {
        mat4 u_projection;
        mat4 u_view;
        vec3 u_eye_pos;
    };
    layout (std140) uniform JointBlock
    {
        mat4 u_jointMatrix[24];
    };
    uniform mat4 u_model;

    out vec3 position;
    out vec3 normal;
    out vec2 texCoords;

    void main()
    {
        mat4 skinMatrix = u_jointMatrix[int(aJoints.x)] * aWeights.x
            + u_jointMatrix[int(aJoints.y)] * aWeights.y
            + u_jointMatrix[int(aJoints.z)] * aWeights.z
            + u_jointMatrix[int(aJoints.w)] * aWeights.w;
        mat4 world = u_model * skinMatrix;
        position = vec3(world * vec4(aPos, 1.0));
        normal = normalize(mat3(world) * aNormal);
        texCoords = aTexCoords;
        gl_Position = u_projection * u_view * vec4(position, 1.0);
    }
)";

const char* fragmentSource = LIX_SHADER_VERSION R"(
    precision highp float;

    in vec2 texCoords;

    out vec4 fragColor;

    uniform sampler2D u_texture;
    uniform vec4 u_base_color;

    void main()
    {
        vec4 tex = texture(u_texture, texCoords);
        fragColor = u_base_color * tex;
        fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2));
    }
)";

static void dumpPolygon(const std::filesystem::path& path, const lix::Polygon& polygon, const std::vector<lix::TRS*>& frames)
{
    std::ofstream ofs(path);
    if(ofs)
    {
        json::Json jo{json::Object};
        json::Json pointsObj{json::Array};
        for(const auto& p : polygon.points())
        {
            json::Json arr{json::Array};
            arr.put(p.x);
            arr.put(p.y);
            arr.put(p.z);
            pointsObj.insert(arr);
        }
        jo.add("points", pointsObj);
        json::Json framesObj{json::Array};
        for(const auto& trs : frames)
        {
            json::Json frameObj{json::Object};
            json::Json transObj{json::Array};
            transObj.put(trs->translation().x);
            transObj.put(trs->translation().y);
            transObj.put(trs->translation().z);
            json::Json rotObj{json::Array};
            rotObj.put(trs->rotation().w);
            rotObj.put(trs->rotation().x);
            rotObj.put(trs->rotation().y);
            rotObj.put(trs->rotation().z);
            json::Json scaleObj{json::Array};
            scaleObj.put(trs->scale().x);
            scaleObj.put(trs->scale().y);
            scaleObj.put(trs->scale().z);
            frameObj.add("translation", transObj);
            frameObj.add("rotation", rotObj);
            frameObj.add("scale", scaleObj);
            framesObj.insert(frameObj);
        }
        jo.add("frames", framesObj);
        ofs << jo << '\n';
    }
    ofs.close();
}

template <typename T>
void dumpBytes(std::ofstream& ofs, const T& t)
{
    char* ptr = (char*)&t;
    for(size_t i{0}; i < sizeof(T); ++i)
    {
        ofs << ptr[i];
    }
}

void dumpBytes(std::ofstream& ofs, char* ptr, size_t len)
{
    for(size_t i{0}; i < len; ++i)
    {
        ofs << ptr[i];
    }
}

void dumpBinary(const std::filesystem::path& path, const lix::Polygon& polygon, const std::vector<lix::TRS*>& frames)
{
    std::ofstream ofs(path, std::ios::binary);
    if(ofs)
    {
        const char* cp = (char*)polygon.points().data();
        const size_t n = polygon.points().size();
        const size_t numBytes = n * sizeof(glm::vec3);
        dumpBytes(ofs, n);
        dumpBytes(ofs, (char*)polygon.points().data(), numBytes);
        const size_t n2 = frames.size();
        dumpBytes(ofs, n2);
        for(lix::TRS* trs : frames)
        {
            dumpBytes(ofs, trs->translation());
            dumpBytes(ofs, trs->rotation());
            dumpBytes(ofs, trs->scale());
        }
        ofs << (unsigned char)0xAB;
    }
    ofs.close();
}

struct Entity
{
    std::shared_ptr<lix::Node> node;
    lix::RigidBody* rigidBody;
    std::shared_ptr<lix::CharacterController> characterController;
    lix::SkinAnimationIterator currentAnimation;
};

struct Consumable
{
    std::shared_ptr<lix::Node> node;
    std::shared_ptr<lix::Shape> shape;
    const glm::vec3 origo;
    bool consumed;
};

struct Platform
{
    lix::ConvexHull convexHull;
    std::shared_ptr<lix::StaticBody> rigidBody;
};

class App : public lix::Application, public lix::Editor
{
public:
    App() : lix::Application{WINDOW_X, WINDOW_Y, "example_platformer"}, lix::Editor{ PERSPECTIVE, RESOLUTION, CAMERA_DISTANCE }
    {}

    virtual void init() override;
    virtual void tick(float dt) override;
    virtual void draw() override;

    virtual void onKeyDown(lix::KeySym key, lix::KeyMod mod) override;
    virtual void onKeyUp(lix::KeySym key, lix::KeyMod mod) override;
    virtual void onMouseMove(float x, float y, float xrel, float yrel) override;

    virtual void onSubjectTransformed(lix::Node* subject, Transformation transformation) override;

    lix::NodePtr createNode(const gltf::Mesh& gltfMesh, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
    {
        auto node = std::make_shared<lix::Node>(
            position,
            glm::dot(rotation, rotation) > FLT_EPSILON
                ? glm::angleAxis(glm::radians(rotation.x), glm::vec3{1.0f, 0.0f, 0.0f})
                    * glm::angleAxis(glm::radians(rotation.y), glm::vec3{0.0f, 1.0f, 0.0f})
                    * glm::angleAxis(glm::radians(rotation.z), glm::vec3{0.0f, 0.0f, 1.0f})
                : glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            scale);
        auto mesh = gltf::loadMesh(gltfMesh);
        node->setMesh(mesh);
        return node;
    }

    Entity& emplaceEntity(std::shared_ptr<lix::Node> node, lix::RigidBody* rigidBody)
    {
        entities.push_back({node, rigidBody});
        return entities.back();
    }

    Consumable& emplaceConsumable(std::shared_ptr<lix::Node> node, std::shared_ptr<lix::Shape> shape)
    {
        consumables.push_back({node, shape, node->translation(), false});
        return consumables.back();
    }

    std::shared_ptr<lix::CharacterController> emplaceCharacterController(std::shared_ptr<lix::Shape> shape)
    {
        return characterControllers.emplace_back(new lix::CharacterController(std::make_shared<lix::DynamicBody>(shape, 1.0f, lix::cubeInertiaTensor(1.0f, 1.0f))));
    }

    std::shared_ptr<lix::StaticBody> emplaceStaticBody(std::shared_ptr<lix::Shape> shape)
    {
        return staticBodies.emplace_back(new lix::StaticBody(shape));
    }

    std::shared_ptr<lix::Sphere> createSphereCollider(lix::Node& node, const gltf::Mesh& gltfMesh)
    {
        std::vector<glm::vec3> vertices;
        std::vector<unsigned int> indices;
        gltf::loadAttributes(gltfMesh, 0, gltf::A_POSITION, vertices, indices);

        glm::mat4 m = glm::mat4(glm::mat3(node.modelMatrix()));
        for(auto& v : vertices)
        {
            v = glm::vec3(m * glm::vec4{v, 1.0f});
        }
        auto [min, max] = lix::extremePoints(vertices);
        glm::vec3 extent = max - min;
        float radii = extent.y * 0.5f; //std::max(extent.x, std::max(extent.y, extent.z)) * 0.5f;
        printf("radii=%.2f\n", radii);

        auto sphere = std::make_shared<lix::Sphere>(&node, radii);

        auto aabb = std::make_shared<lix::AABB>(&node, min, max);
        sphere->setSimplified(aabb);
        
        return sphere;
    }

    std::shared_ptr<lix::Polygon> createMeshCollider(lix::Node& node, const gltf::Mesh& gltfMesh)
    {
        auto poly = gltf::loadMeshCollider(gltfMesh, true);
        poly->setTRS(&node);

        auto aabb = std::make_shared<lix::AABB>(poly->trs(), poly.get());
        poly->setSimplified(aabb);
        return poly;
    }

    std::shared_ptr<Platform> getPlatform(std::shared_ptr<lix::StaticBody> staticBody)
    {
        std::shared_ptr<Platform> rval{nullptr};
        for(auto& platform : platforms)
        {
            if(platform && platform->rigidBody.get() == staticBody.get())
            {
                rval = platform;
                break;
            }
        }
        return rval;
    }

    lix::ShaderProgramPtr objectShader;
    lix::ShaderProgramPtr skinShader;
    std::vector<Entity> entities;
    std::vector<Consumable> consumables;
    std::vector<std::shared_ptr<lix::CharacterController>> characterControllers;
    std::array<std::shared_ptr<Platform>, 3> platforms;
    std::vector<std::shared_ptr<lix::StaticBody>> staticBodies;
    std::shared_ptr<lix::VAO> closestFaceVAO;
    std::shared_ptr<lix::VAO> platformConvexVAO;
    bool xrayMode{false};
};

int main(int argc, char* argv[])
{
    App app;
    app.run();
    return 0;
}

void printDuration(const lix::SkinAnimation& skinAnimation)
{
    printf("%s: %.1fs\n", skinAnimation.name().c_str(), (skinAnimation.end() - skinAnimation.start()));
}

void App::init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    setInputAdapter(this);

    objectShader.reset(new lix::ShaderProgram(
        vertexSource,
        fragmentSource
    ));
    skinShader.reset(new lix::ShaderProgram(
        vertexSkinSource,
        fragmentSource
    ));
    camera().setupUBO({
        objectShader.get(),
        skinShader.get()
    });

    auto node = gltf::loadNode(assets::objects::macman::Armature_node);
    node->setTranslation(glm::vec3{4.0f, 3.0f, -6.0f});
    gltf::loadSkin(node.get(), assets::objects::macman::Armature_skin);
    printDuration(*gltf::loadAnimation(node.get(), assets::objects::macman::Idle_anim));
    printDuration(*gltf::loadAnimation(node.get(), assets::objects::macman::Running_anim));
    printDuration(*gltf::loadAnimation(node.get(), assets::objects::macman::Jump_anim));
    printDuration(*gltf::loadAnimation(node.get(), assets::objects::macman::Leaping_anim));
    printDuration(*gltf::loadAnimation(node.get(), assets::objects::macman::StrafingLeft_anim));
    printDuration(*gltf::loadAnimation(node.get(), assets::objects::macman::StrafingRight_anim));

    auto cctrl = emplaceCharacterController(createSphereCollider(*node, assets::objects::macman::Cube_001_mesh));
    auto& ent = emplaceEntity(node, cctrl->dynamicBody.get());
    ent.currentAnimation = node->skin()->animations().begin();
    ent.characterController = cctrl;


    /*auto donut = gltf::loadNode(assets::objects::donut::Donut_node);
    emplaceEntity(donut, nullptr);//emplaceStaticBody(createSphereCollider(*node, assets::objects::donut::Torus_003_mesh)).get());
    donut->setTranslation((glm::vec3{-6.0f, 2.0f, -12.0f} + glm::vec3{4.0f, 1.0f, -6.0f}) * 0.5f + glm::vec3{1.5f, 2.5f, 1.5f});
    donut->setScale(glm::vec3{4.0f, 4.0f, 4.0f});*/

    for(const auto& obj : assets::levels::level::nodes)
    {
        auto node = gltf::loadNode(obj);
        if(obj.mesh->name == "Donut")
        {
            emplaceConsumable(node, createSphereCollider(*node, *obj.mesh));
        }
        else
        {
            emplaceEntity(node, emplaceStaticBody(createMeshCollider(*node, *obj.mesh)).get());
        }
    }

    /*createMeshCollider(assets::objects::spyro_ps1::mesh0_001_mesh, true, glm::vec3{0.0f, 6.0f, 0.0f});//, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.2f, 0.8f, 0.1f});
    createMeshCollider(assets::objects::platform::Cube_mesh, false, glm::vec3{0.0f, 0.0f, 0.0f});
    createMeshCollider(assets::objects::platform::Cube_mesh, false, glm::vec3{0.0f, 4.0f, -16.0f});
    createMeshCollider(assets::objects::platform::Cube_mesh, false, glm::vec3{4.0f, 1.0f, -6.0f});
    createMeshCollider(assets::objects::platform::Cube_mesh, false, glm::vec3{-6.0f, 2.0f, -12.0f}, glm::vec3{0.0f, 45.0f, 0.0f}, glm::vec3{1.0f, 1.0f, 3.0f});*/
}

void App::tick(float dt)
{
    //refresh(dt);
    float t = lix::Time::seconds();

    for(auto& consumable : consumables)
    {    
        consumable.node->setTranslation({consumable.origo.x, consumable.origo.y + sinf(t * 4.0f) * 0.1f, consumable.origo.z});
        consumable.node->setRotation(
            glm::angleAxis(t * 1.0f, glm::vec3{0.0f, 1.0f, 0.0f})
            * glm::angleAxis(sinf(t * 0.7f) * 0.3f, glm::vec3{1.0f, 0.0f, 0.0f})
        );
    }

    static lix::Timer timerSeconds{lix::Time::fromSeconds(2)};
    if(!timerSeconds.elapsed())
    {
        return;
    }

    for(const auto& ent : entities)
    {
        if(ent.node->skin())
        {
            ent.currentAnimation->second->update(dt);
        }
    }

    auto& playerCtrl = characterControllers.front();
    const glm::vec3 playerPos = playerCtrl->dynamicBody->shape->trs()->translation();
    camera().setTarget(playerPos);
    camera().setTranslation(playerPos - glm::vec3{sinf(playerCtrl->yaw) * 8.0f, -4.0f, cosf(playerCtrl->yaw) * 8.0f});
    camera().refresh(dt);

    if(playerPos.y < -10.0f) // respawn
    {
        playerCtrl->dynamicBody->shape->trs()->setTranslation(glm::vec3{0.0f, 6.0f, 0.0f});
        playerCtrl->dynamicBody->velocity = glm::vec3{0.0f};
    }

    static lix::Collision collision;
    for(auto& cctrl : characterControllers)
    {
        auto dyn = cctrl->dynamicBody.get();
        if(cctrl->deltaControl.y > 0)
        {
            dyn->shape->trs()->applyTranslation(glm::vec3{0.0f, 0.05f, 0.0f});
            dyn->velocity.y = 6.0f;
            cctrl->deltaControl.y = 0.0f;
            cctrl->platform.reset();
        }
        if(cctrl->platform)
        {
            auto& v = cctrl->dynamicBody->velocity;
            if(v.x * v.x + v.z * v.z > 0.01f)
            {
                static std::shared_ptr<Platform> prevPlatform{nullptr};
                auto platform = getPlatform(cctrl->platform);
                static std::list<lix::Face>::const_iterator prevClosestFace = platform->convexHull.end();
                const glm::vec3 p = cctrl->dynamicBody->shape->trs()->translation() - cctrl->platform->shape->trs()->translation();
                auto closestFace = platform->convexHull.closestFace(p);
                float dp = glm::dot(closestFace->normal, glm::vec3{0.0f, 1.0f, 0.0f});
                dyn->shape->trs()->setRotation(glm::slerp(dyn->shape->trs()->rotation(),
                    glm::angleAxis(cctrl->yaw, glm::vec3{0.0f, 1.0f, 0.0f}), 4.0f * dt));
                if(glm::dot(closestFace->normal, p - closestFace->half_edge->vertex) > 0 && dp < 0.7f)
                {
                    //printf("dp=%.2f\n", dp);
                    cctrl->dynamicBody->velocity += (closestFace->normal + glm::vec3{0.0f, 1.0f, 0.0f}) * 0.1f;
                    cctrl->platform.reset();
                }
                if(prevClosestFace != closestFace)
                {
                    closestFaceVAO.reset(new lix::VAO(lix::Attributes{lix::Attribute::VEC3}, std::vector<glm::vec3>{
                        closestFace->half_edge->vertex,
                        closestFace->half_edge->next->vertex,
                        closestFace->half_edge->next->next->vertex,
                    }));
                    prevClosestFace = closestFace;
                }
                if(prevPlatform != platform)
                {
                    platformConvexVAO.reset(new lix::VAO(lix::Attributes{lix::Attribute::VEC3}, 
                        platform->convexHull.points()
                    ));
                    prevPlatform = platform;
                }
            }
        }
        else
        {
            // apply gravity
            dyn->velocity.y += -9.82f * dt;
            static lix::TRS lastTRS;
            for(auto& staticBody : staticBodies)
            {
                if(lix::collides(*dyn->shape, *staticBody->shape, &collision))
                {
                    /*if(collision.penetrationDepth > 0.1f)
                    {
                        pause();
                        lix::TRS nextTRS{*dyn->shape->trs()};
                        nextTRS.applyTranslation(collision.normal * (collision.penetrationDepth));
                        if(auto poly = dynamic_cast<lix::Polygon*>(dyn->shape.get()))
                        {
                            dumpPolygon("poly_001.json", *poly, {&lastTRS, poly->trs(), &nextTRS});
                            dumpBinary("poly_001.bin", *poly, { poly->trs(), &nextTRS});
                        }
                        if(auto poly = dynamic_cast<lix::Polygon*>(staticBody->shape.get()))
                        {
                            dumpPolygon("poly_002.json", *poly, {poly->trs()});
                            dumpBinary("poly_002.bin", *poly, {poly->trs()});
                        }
                        printf("pos y=%.2f pen=%.2f\n", dyn->shape->trs()->translation().y, collision.penetrationDepth);
                    }*/
                    dyn->velocity.y = 0.0f;
                    dyn->shape->trs()->applyTranslation(collision.normal * (collision.penetrationDepth));
                    //printf("pos y=%.2f\n", dyn->shape->trs()->translation().y);
                    cctrl->platform = staticBody;
                    bool loaded{false};
                    auto platform = getPlatform(staticBody);
                    if(platform == nullptr)
                    {
                        printf("created platform convex\n");
                        for(size_t i{platforms.size() - 1}; i > 0; --i)
                        {
                            std::swap(platforms[i], platforms[i - 1]);
                        }
                        auto unique = lix::uniqueVertices(std::dynamic_pointer_cast<lix::Polygon>(staticBody->shape)->transformedPoints());
                        for(auto& u : unique)
                        {
                            u -= staticBody->shape->trs()->translation();
                        }
                        platforms[0].reset(new Platform{unique, staticBody});
                        platform = platforms[0];
                    }
                    //break;
                }
            }

            lastTRS.setTranslation(dyn->shape->trs()->translation());
            lastTRS.setRotation(dyn->shape->trs()->rotation());
            lastTRS.setScale(dyn->shape->trs()->scale());
        }
    }

    for(auto& cctrl : characterControllers)
    {
        auto dyn = cctrl->dynamicBody.get();
        glm::vec3 x0z = glm::normalize(glm::vec3{-cctrl->deltaControl.x, 0.0f, -cctrl->deltaControl.z});
        x0z = dyn->shape->trs()->rotation() * x0z;
        float acc = cctrl->onGround() ? 16.0f : 2.0f;
        float dacc = cctrl->onGround() ? 12.0f : 1.0f;
        if(glm::dot(x0z, x0z) > lix::EPSILON)
        {
            dyn->velocity.x = glm::mix(dyn->velocity.x, x0z.x * cctrl->maxWalkSpeed, acc * dt);
            dyn->velocity.z = glm::mix(dyn->velocity.z, x0z.z * cctrl->maxWalkSpeed, acc * dt);
        }
        else
        {
            if (dyn->velocity.x * dyn->velocity.x > 0.0001f) { dyn->velocity.x = glm::mix(dyn->velocity.x, 0.0f, dacc * dt); }
            else { dyn->velocity.x = 0.0f; }
            if (dyn->velocity.z * dyn->velocity.z > 0.0001f) { dyn->velocity.z = glm::mix(dyn->velocity.z, 0.0f, dacc * dt); }
            else { dyn->velocity.z = 0.0f; }
        }
        dyn->shape->trs()->applyTranslation(dyn->velocity * dt);
        cctrl->updateMovementState();
    }

    for(auto& ent : entities)
    {
        if(ent.node->skin()) {
            if(ent.characterController)
            {
                switch(ent.characterController->movementState)
                {
                case lix::CharacterController::MovementState::IDLE:
                    ent.currentAnimation = ent.node->skin()->animations().find("Idle");
                    break;
                case lix::CharacterController::MovementState::JUMPING:
                    ent.currentAnimation = ent.node->skin()->animations().find("Leaping");
                    break;
                case lix::CharacterController::MovementState::FALLING:
                    ent.currentAnimation = ent.node->skin()->animations().find("Jump");
                    break;
                case lix::CharacterController::MovementState::FORWARD:
                    ent.currentAnimation = ent.node->skin()->animations().find("Running");
                    break;
                case lix::CharacterController::MovementState::LEFT:
                    ent.currentAnimation = ent.node->skin()->animations().find("StrafingLeft");
                    break;
                case lix::CharacterController::MovementState::RIGHT:
                    ent.currentAnimation = ent.node->skin()->animations().find("StrafingRight");
                    break;
                }
            }
        }
    }
}

void App::draw()
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    objectShader->bind();
    if(xrayMode) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
    for(const auto& ent : entities)
    {
        if(ent.node->skin())
        {
            continue;
        }
        lix::renderNode(*objectShader, *ent.node);
    }
    for(const auto& consumable : consumables)
    {
        lix::renderNode(*objectShader, *consumable.node);
    }
    skinShader->bind();
    for(const auto& ent : entities)
    {
        if(ent.node->skin())
        {
            lix::renderSkinAnimationNode(*skinShader, *ent.node);
        }
    }
    
    if(xrayMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        //glClear(GL_DEPTH_BUFFER_BIT);
        if(closestFaceVAO)
        {
            objectShader->setUniform("u_model", glm::mat4{1.0f});
            objectShader->setUniform("u_base_color", lix::Color::red.vec4());
            closestFaceVAO->bind();
            closestFaceVAO->draw();
        }
        if(platformConvexVAO)
        {
            objectShader->setUniform("u_model", glm::mat4{1.0f});
            objectShader->setUniform("u_base_color", lix::Color::yellow.vec4() * glm::vec4{1.0f, 1.0f, 1.0f, 0.5f});
            platformConvexVAO->bind();
            platformConvexVAO->draw();
        }
    }
}

void App::onSubjectTransformed(lix::Node* subject, lix::Editor::Transformation transformation)
{

}

void App::onKeyDown(lix::KeySym key, lix::KeyMod mod)
{
    lix::Editor::onKeyDown(key, mod);
    auto& player = characterControllers.front();
    switch(key)
    {
        case SDLK_SPACE:
            player->jump();
            break;
        case SDLK_w:
            player->moveForward();
            break;
        case SDLK_a:
            player->moveLeft();
            break;
        case SDLK_s:
            player->moveBackward();
            break;
        case SDLK_d:
            player->moveRight();
            break;
        case SDLK_x:
            xrayMode = !xrayMode;
            break;
    }
}

void App::onKeyUp(lix::KeySym key, lix::KeyMod mod)
{
    lix::Editor::onKeyUp(key, mod);
    auto& player = characterControllers.front();
    switch(key)
    {
        case SDLK_SPACE:
            player->stopJump();
            break;
        case SDLK_w:
            player->stopForward();
            break;
        case SDLK_a:
            player->stopLeft();
            break;
        case SDLK_s:
            player->stopBackward();
            break;
        case SDLK_d:
            player->stopRight();
            break;
    }
}

void App::onMouseMove(float x, float y, float xrel, float yrel)
{
    lix::Editor::onMouseMove(x, y, xrel, yrel);
    //static float prevX = x;
    //characterControllers.front()->yaw += (prevX - x) / WINDOW_X * 4.0f;
    //prevX = x;
    characterControllers.front()->rotate(-xrel / WINDOW_X * 4.0f);
}