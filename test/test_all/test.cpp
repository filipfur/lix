#include <iostream>
#include <cmath>
#include <string>
#include <fstream>

#include <SDL2/SDL.h>

#include "glm/glm.hpp"

#include "glshaderprogram.h"
#include "glvertexarray.h"
#include "gltexture.h"
#include "gltfloader.h"
#include "glmesh.h"
#include "glapplication.h"
#include "glnode.h"
#include "gluniformbuffer.h"
#include "glframebuffer.h"

#include "glrendering.h"
#include "glinstancedrendering.h"
#include "glanimation.h"
#include "glskinanimation.h"
#include "glfont.h"
#include "gltext.h"
#include "gltextrendering.h"
#include "glblurpass.h"
#include "glerror.h"
#include "collision.h"
#include "polygon.h"
#include "glpolygonrendering.h"

#include "ecssystem.h"
#include "ecsentity.h"
#include "ecscomponent.h"

#include "gen/shaders/anim_vert.h"
#include "gen/shaders/object_vert.h"
#include "gen/shaders/texture_object_frag.h"
#include "gen/shaders/inst_vert.h"
#include "gen/shaders/screen_vert.h"
#include "gen/shaders/screen_frag.h"
#include "gen/shaders/blur_frag.h"
#include "gen/shaders/bloom_frag.h"
#include "gen/shaders/text_vert.h"
#include "gen/shaders/text_wavy_vert.h"
#include "gen/shaders/text_frag.h"
#include "gen/shaders/text_border_frag.h"
#include "gen/shaders/testtypes_vert.h"
#include "gen/shaders/testtypes_frag.h"
#include "gen/shaders/line_vert.h"
#include "gen/shaders/line_frag.h"
#include "gen/images/tex_png.h"
#include "gen/objects/cube.h"
#include "gen/objects/bush.h"
#include "gen/objects/moose.h"
#include "gen/objects/monkey_stage.h"
#include "gen/objects/donut.h"
#include "gen/objects/bone.h"
#include "gen/objects/planets.h"
#include "gen/objects/spikes.h"
#include "gen/objects/animated_cube.h"
#include "gen/fonts/arial.h"

#define LO_RES_X 720
#define LO_RES_Y 480

#define MID_RES_X 1080
#define MID_RES_Y 720

#define HI_RES_X 1400
#define HI_RES_Y 800

#define SCREEN_WIDTH MID_RES_X
#define SCREEN_HEIGHT MID_RES_Y

#define LOGONCE(msg) do { static bool _logOnceFlag = true; if (_logOnceFlag) { std::cout << (msg) << std::endl; _logOnceFlag = false; } } while(0)

#define RUNONCE(actions) do { static bool _runOnceFlag = true; if (_runOnceFlag) { actions; _runOnceFlag = false; } } while(0)

const float aspectRatio = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);

void unitTest()
{
    std::cout << "unitTesting()" << std::endl;

    lix::Color a{1337.0f, 666.0f, 50.0f};
    lix::Color b{glm::vec3{300.0f, 45.0f, 12.0f}};

    lix::Color c = std::move(b);
    assert(c.vec4().y == 45.0f);
    b = std::move(c);
    assert(b.vec4().z == 12.0f);

    lix::Material mat{b};
    lix::Material mat2{mat};
    mat2.setBaseColor(mat.baseColor());
    mat2.setMetallic(mat.metallic());
    mat2.setRoughness(mat.roughness());
    mat2.setNormalMap(mat.normalMap());
    mat2.setArmMap(mat.armMap());

    lix::FBO::getFramebufferStatusString(GL_FRAMEBUFFER_UNDEFINED);
    lix::FBO::getFramebufferStatusString(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
    lix::FBO::getFramebufferStatusString(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
    lix::FBO::getFramebufferStatusString(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
    lix::FBO::getFramebufferStatusString(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
    lix::FBO::getFramebufferStatusString(GL_FRAMEBUFFER_UNSUPPORTED);
    lix::FBO::getFramebufferStatusString(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);

    lix::traceGLError("traceGLError_test", GL_INVALID_OPERATION);

    lix::TRS trs_a{glm::vec3{13.0f, 0.0f, 0.0f}, glm::quat{1.0f, 0.0f, 0.0f, 0.0f}, glm::vec3{1.0f}};
    lix::TRS trs_b;
    trs_b = trs_a;
    assert(trs_b.applyTranslation(glm::vec3{1.0f})->rotation().w == 1.0f);
}

struct Time {
    float time;
    float deltaTime;
};

struct Actor {
    impact::Shape* shape;
    bool moveable;
    glm::vec3 velocity;
};

namespace Component
{
    using Time = ecs::Component<Time,0,true>;
    using Actor = ecs::Component<Actor>;
};

class App : public lix::Application, public lix::InputAdapter
{
public:
    App(int windowX, int windowY, const char* title) : Application{windowX, windowY, title} {}

    virtual void init() override;

    virtual void tick(float dt) override;

    virtual void draw() override;

    virtual void onKeyDown(lix::KeySym key, lix::KeyMod mod) override;
    virtual void onKeyUp(lix::KeySym key, lix::KeyMod mod) override;
    virtual void onMouseDown(lix::KeySym key, lix::KeyMod mod) override;
    virtual void onMouseUp(lix::KeySym key, lix::KeyMod mod) override;

private:
    void renderModels();
    void renderTest();
    void renderTerrainInstances();
    void renderBoneInstances();
    void renderSkinnedModels();
    void renderText();
    void renderPolygons();

    glm::mat4 model{1.0f};
    std::shared_ptr<lix::ShaderProgram> shaderProgram;
    std::shared_ptr<lix::ShaderProgram> animShader;
    std::shared_ptr<lix::ShaderProgram> instShader;
    std::shared_ptr<lix::ShaderProgram> customBlurShader;
    std::shared_ptr<lix::ShaderProgram> testShader;
    std::shared_ptr<lix::ShaderProgram> screenShader;
    lix::NodePtr mooseNode;
    std::shared_ptr<lix::SkinAnimation> mooseAnim;
    std::shared_ptr<lix::Node> monkeyStageNode;
    std::shared_ptr<lix::Node> planetsNode;
    std::shared_ptr<lix::Node> animatedCubeNode;
    std::shared_ptr<lix::SkinAnimation> cubeAnim;
    lix::NodePtr spikesNode;
    lix::NodePtr donutNode;
    lix::MeshPtr bushMesh;
    lix::MeshPtr boneMesh;
    lix::MeshPtr quadMesh;
    std::vector<ecs::Entity> entities;
    std::shared_ptr<impact::Polygon> rectangle;
    std::shared_ptr<impact::Polygon> circle;
    std::shared_ptr<impact::Polygon> circle2;
    std::shared_ptr<lix::FrameBuffer> postFBO;
    std::shared_ptr<lix::FrameBuffer> blitFBO;
    std::shared_ptr<lix::Texture> texture;
    std::shared_ptr<lix::Font> arialFont;
    const glm::vec3 upVector{0.0f, 1.0f, 0.0f};
    glm::vec3 cameraPosition{};
    glm::vec3 cameraTarget{0.0f, 14.0f, 0.0f};
    float cameraYaw{glm::pi<float>() * 0.5f};

    const std::vector<glm::vec3> lights = {
        glm::vec3{-14.0f, 8.0f, 10.0f},
        glm::vec3{-20.0f, 20.0f, -4.0f},
        glm::vec3{7.0f, 5.0f, 6.0f}
    };


    struct CameraBlock {
        glm::mat4 projection{glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 200.0f)};
        glm::mat4 view{1.0f};
        glm::vec3 position{0.0f, 0.0f, 0.0f};
        float padding;
    } cameraBlock;

    std::shared_ptr<lix::UniformBuffer> cameraUBO;
};

int main(int argc, char* argv[])
{
    App app{SCREEN_WIDTH, SCREEN_HEIGHT, "Lithium X - Example"};
    const GLubyte *version;
    version = glGetString(GL_VERSION);
    std::cout << "version: " << version << std::endl;
    unitTest();
    app.run();
    return 0;
}

void App::init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    setInputAdapter(this);
    assert(inputAdapter() == this);

    shaderProgram.reset(new lix::ShaderProgram(assets::shaders::object_vert,
        assets::shaders::texture_object_frag));
    testShader.reset(new lix::ShaderProgram(assets::shaders::testtypes_vert,
        assets::shaders::testtypes_frag));
    animShader.reset(new lix::ShaderProgram(assets::shaders::anim_vert,
        assets::shaders::texture_object_frag));
    instShader.reset(new lix::ShaderProgram(assets::shaders::inst_vert,
        assets::shaders::texture_object_frag));
    screenShader.reset(new lix::ShaderProgram(assets::shaders::screen_vert,
        assets::shaders::bloom_frag));
    screenShader->bind();
    screenShader->setUniform("u_bright", 1);
    customBlurShader.reset(new lix::ShaderProgram(assets::shaders::screen_vert,
        assets::shaders::blur_frag));
    customBlurShader->bind();
    customBlurShader->setUniform("horizontal", false);

    static const std::vector<glm::vec3> vert_0 = {
        glm::vec3{1.0f, 1.0f, 0.0f},
        glm::vec3{5.0f, 1.0f, 0.0f},
        glm::vec3{5.0f, 3.0f, 0.0f},
        glm::vec3{1.0f, 3.0f, 0.0f}
    };
    rectangle.reset(
        new impact::Polygon(vert_0)
    );
    rectangle->setPosition(glm::vec3{2.0f, 0.0f, 0.0f})
        ->setRotation(glm::angleAxis(glm::pi<float>() * 0.05f, glm::vec3{0.0f, 0.0f, 1.0f}))
        ->setScale(glm::vec3{4.0f});

    std::vector<glm::vec3> vert_1;
    float ratio = glm::pi<float>() / 16.0f;
    for(int i{0}; i < 32; ++i)
    {
        float rad = static_cast<float>(i) * ratio;
        vert_1.push_back(glm::vec3{glm::cos(rad) * 1.0f, glm::sin(rad) * 1.0f, 0.0f});
    }
    circle.reset(
        new impact::Polygon(vert_1)
    );
    circle->setPosition(glm::vec3{12.0f, 20.0f, 0.0f})
        ->setScale(glm::vec3{2.0f});

    circle2.reset(
        new impact::Polygon(vert_1)
    );
    circle2->setPosition(glm::vec3{13.0f, 26.0f, 0.0f})
        ->setScale(glm::vec3{1.4f});

    for(auto& poly : {rectangle, circle, circle2})
    {
        ecs::Entity entity = entities.emplace_back(ecs::EntityRegistry::createEntity());
        ecs::attach<Component::Time, Component::Actor>(entity);
        Actor& actor = Component::Actor::get(entity);
        actor.moveable = poly != rectangle;
        actor.shape = poly.get();
        actor.velocity = glm::vec3{0.0f, -1.0f, 0.0f};
    }
    
    monkeyStageNode = gltf::loadNode(assets::objects::monkey_stage::Cube_node);
    monkeyStageNode->setTranslation(glm::vec3{20.0f, 0.0f, -40.0f});
    monkeyStageNode->setScale(glm::vec3(4.0f));
    boneMesh = gltf::loadMesh(assets::objects::bone::Cube_mesh);
    donutNode = gltf::loadNode(assets::objects::donut::Torus_node);
    bushMesh = gltf::loadMesh(assets::objects::bush::Plane_003_mesh);
    planetsNode = gltf::loadNode(assets::objects::planets::Icosphere_001_node);
    planetsNode->setTranslation(glm::vec3{-20.0f, 20.0f, -4.0f});
    planetsNode->setScale(glm::vec3{2.0f});
    spikesNode = gltf::loadNode(assets::objects::spikes::Cone_node);
    spikesNode->setTranslation(glm::vec3{-14.0f, 0.0f, 10.0f});
    
    arialFont.reset(new lix::Font(assets::fonts::arial::create()));

    mooseNode = gltf::loadNode(assets::objects::moose::Armature_node);
    animatedCubeNode = gltf::loadNode(assets::objects::animated_cube::Armature_node);
    animatedCubeNode->setTranslation(glm::vec3{4.0f, 0.0f, 6.0f});
    animatedCubeNode->setScale(glm::vec3{4.0f});
    gltf::loadSkin(animatedCubeNode.get(), assets::objects::animated_cube::Armature_skin);
    cubeAnim = gltf::loadAnimation(animatedCubeNode.get(), assets::objects::animated_cube::ArmatureAction_anim);

    postFBO.reset(new lix::FrameBuffer(glm::ivec2{SCREEN_WIDTH, SCREEN_HEIGHT}));
    postFBO->bind();
    postFBO->createTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    postFBO->createTexture(GL_COLOR_ATTACHMENT1, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    postFBO->createRenderBuffer(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8);
    postFBO->bindColorsAsDrawBuffers();
    postFBO->unbind();

    blitFBO.reset(new lix::FrameBuffer(glm::ivec2{SCREEN_WIDTH, SCREEN_HEIGHT}));
    blitFBO->bind();
    blitFBO->createTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    blitFBO->unbind();

    cameraUBO.reset(new lix::UniformBuffer(
        sizeof(CameraBlock), (void*)&cameraBlock, "CameraBlock", 0, GL_STATIC_DRAW
    ));
    cameraUBO->bindShaders({
        shaderProgram.get(),
        animShader.get(),
        instShader.get(),
        testShader.get()
    });

    gltf::loadSkin(mooseNode.get(), assets::objects::moose::Armature_skin);
    mooseAnim = gltf::loadAnimation(mooseNode.get(), assets::objects::moose::ArmatureAction_001_anim);

    std::cout << "moose anim start=" << mooseAnim->start() << ", end=" << mooseAnim->end() << ", time=" << mooseAnim->time() << std::endl;

    texture.reset(new lix::Texture(
        assets::images::tex_png_rgba::data,
        assets::images::tex_png_rgba::width,
        assets::images::tex_png_rgba::height,
        GL_UNSIGNED_BYTE, GL_RGBA, GL_RGBA));

    texture->bind();
    texture->setWrap(GL_CLAMP_TO_EDGE);

    texture->generateMipmap();
    texture->setFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    setOnKeyDown(SDLK_ESCAPE, [this](auto, auto) {
        this->quit();
    });

    setOnKeyUp(SDLK_ESCAPE, [](auto, auto) {

    });

    setOnMouseDown(SDL_BUTTON_LEFT, [](auto, auto) {

    });

    setOnMouseUp(SDL_BUTTON_LEFT, [](auto, auto) {

    });

    setOnMouseDrag(SDL_BUTTON_LEFT, [this](auto /*key*/, auto mod, auto dragState) {
        static auto lastMP{normalizedMousePosition()};
        auto mp = normalizedMousePosition();
        auto mouseDelta{mp - lastMP};
        lastMP = mp;
        switch(dragState)
        {
            case lix::DragState::START_DRAG:
                break;
            case lix::DragState::DRAGGING:
                if(mod & KMOD_SHIFT)
                {
                    glm::vec3 right = glm::normalize(glm::cross(cameraTarget - cameraBlock.position, upVector));
                    glm::vec3 forward = glm::cross(right, upVector);
                    glm::vec3 translation = -right * mouseDelta.x + -forward * mouseDelta.y;
                    cameraTarget += translation * 64.0f;
                    cameraPosition += translation * 64.0f;
                }
                else
                {
                    cameraYaw += mouseDelta.x * 4.0f;
                }
                break;
            case lix::DragState::END_DRAG:
                break;
        }
    });
}

void App::tick(float dt)
{
    cameraBlock.position = cameraPosition + glm::vec3{cosf(cameraYaw) * 36.0f, 42.0f, sinf(cameraYaw) * 36.0f};
    cameraBlock.view = glm::lookAt(cameraBlock.position,
                    cameraTarget,
                    upVector);

    static lix::Animation animation{"Test", {
        {1, 0.5f},
        {2, 1.0f},
        {3, 0.5f},
        {1000, 4.0f}
    }};

    static ecs::System<Component::Actor> collisionSystem;
    collisionSystem.update(entities, [this](ecs::Entity entityA, Actor& actorA){
        ecs::Slice<Component::Actor>::forEach(entities, [&entityA, &actorA](ecs::Entity entityB, Actor& actorB){
            if(entityA >= entityB)
            {
                return;
            }
            impact::Shape& shapeA = *actorA.shape;
            impact::Shape& shapeB = *actorB.shape;

            std::vector<glm::vec3> simplex;
            const glm::vec3 D{shapeB.position() - shapeA.position()};
            if(impact::gjk(shapeA, shapeB, simplex, D))
            {
                glm::vec3 collisionVector;
                float penetration;
                if(impact::epa(shapeA, shapeB, simplex, collisionVector, penetration))
                {
                    if(actorA.moveable)
                    {
                        actorA.velocity = -collisionVector;
                        shapeA.move(-collisionVector * penetration);
                    }
                    if(actorB.moveable)
                    {
                        actorB.velocity = collisionVector;
                        shapeB.move(-collisionVector * penetration);
                    }
                }
            }
        });
    });
    
    if(circle->position().y < 0)
    {
        circle->setPosition(glm::vec3{12.0f, 20.0f, 0.0f});
    }
    static Time worldTime{0, 0};
    worldTime.time += dt;
    worldTime.deltaTime = dt;
    Component::Time::set(worldTime);
    static ecs::System<const Component::Time, Component::Actor> actorSystem;
    actorSystem.update(entities, [](ecs::Entity /*entity*/, const Time& time, Actor& actor) {
        if(actor.moveable)
        {
            actor.shape->move(actor.velocity * time.deltaTime);
        }
    });

    texture->bind();
    //texture->setLodBias(static_cast<float>((static_cast<int>(time()) % 4)));

    planetsNode->applyRotation(glm::angleAxis(dt * 0.16f, glm::vec3{0.0f, 1.0f, 0.0f})
        * glm::angleAxis(dt * -0.08f, glm::vec3{1.0f, 0.0f, 0.0f}));

    static auto earthNode = planetsNode->find("Icosphere");
    earthNode->applyRotation(glm::angleAxis(dt * 0.32f, glm::vec3{1.0f, 0.0f, 0.0f}));

    spikesNode->setScale(glm::vec3{4.0f + sin(time() * 16.0f) * 0.08f});

    mooseAnim->update(dt);
    cubeAnim->update(dt);

    animation.update(dt);
}

void App::renderModels()
{
    shaderProgram->bind();
    shaderProgram->setUniform("u_model", model);
    shaderProgram->setUniform("u_time", time());
    shaderProgram->setUniform("u_lights", lights);

    lix::renderNode(*shaderProgram, *monkeyStageNode);
    lix::renderNode(*shaderProgram, *spikesNode);
    lix::renderNode(*shaderProgram, *planetsNode);

    texture->bind(GL_TEXTURE0);
    static glm::mat4 maModel{glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-8.0f, 28.0f, 4.0f)), glm::vec3{10.0f})};
    shaderProgram->setUniform("u_model", maModel);
    shaderProgram->setUniform("u_base_color", glm::vec4{0.9f, 0.8f, 0.8f, 1.0f});
    lix::renderQuad();

    static lix::Node donutA{
        lix::TRS{glm::vec3{-10.0f, 4.0f, 0.0f},
            glm::angleAxis(glm::pi<float>() * 0.5f, glm::vec3{1.0f, 0.0f, 0.0f}),
            glm::vec3{2.0f}}
    };
    static lix::Node donutB{
        lix::TRS{glm::vec3{-16.0f, 4.0f, 0.0f},
            glm::angleAxis(glm::pi<float>() * 0.5f, glm::vec3{1.0f, 0.0f, 0.0f}),
            glm::vec3{2.0f}}
    };
    donutA.setTranslation(glm::vec3{
        donutA.translation().x,
        4.0f + sinf(time() * 8.0f) * 0.4f,
        donutB.translation().z});
    donutB.setTranslation(glm::vec3{
        donutB.translation().x,
        4.0f + sinf((time() + 0.2f) * 8.0f) * 0.4f,
        donutB.translation().z});

    for(lix::Node* node : {&donutA, &donutB})
    {
        lix::renderMesh(*shaderProgram, *donutNode->mesh(), node->model());
    }
}

void App::renderTest()
{
    testShader->bind();
    static glm::mat3 rotMat = glm::mat4_cast(glm::angleAxis(glm::pi<float>() * 0.25f,
        glm::vec3{0.0f, 0.0f, 1.0f}));
    testShader->setUniform("u_mat3", rotMat);
    testShader->setUniform("u_vec3", glm::vec3{2.0f, 0.0f, 0.0f});
    testShader->setUniform("u_vec2", glm::vec2{0.0f, 2.0f});
    testShader->setUniform("u_ivec3", glm::ivec3{-1, 0, 0});
    testShader->setUniform("u_ivec2", glm::ivec2{0, -1});
    testShader->setUniform("u_base_color", glm::vec4{1.0f, 0.0f, 1.0f, 1.0f});
    static auto redMat = std::make_shared<lix::Material>(
        lix::Color{1.0f, 0.0f, 0.0f}
    );
    static lix::Mesh triangle{
        {lix::Attribute::VEC3, lix::Attribute::VEC3, lix::Attribute::VEC2}, {
            -1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,   1.0f, 1.0f
        },
        GL_TRIANGLES,
        GL_STATIC_DRAW,
        redMat
    };
    static lix::Mesh quad{
        {lix::Attribute::VEC3, lix::Attribute::VEC3, lix::Attribute::VEC2}, {
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
            1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
            1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
            -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f
        },{
            0, 1, 2,
            0, 2, 3
        },
        GL_TRIANGLES,
        GL_STATIC_DRAW,
        nullptr
    };
    static auto blueTex = lix::Texture::Basic(glm::vec3{0.0f,0.0f,1.0f});
    blueTex->bind();
    assert(blueTex->width() == 1);
    assert(blueTex->height() == 1);
    assert(blueTex->type() == GL_UNSIGNED_BYTE);
    assert(blueTex->internalFormat() == GL_RGB8);
    assert(blueTex->colorFormat() == GL_RGB);
    static glm::mat4 triModel{glm::translate(glm::scale(glm::mat4{1.0f}, glm::vec3{8.0f}), glm::vec3{0.0f, 0.0f, -2.0f})};
    lix::renderMesh(*testShader, triangle, triModel);
    assert(quad.count() == 1);
    assert(quad.primitive(0).vao);
    testShader->setUniform("u_base_color", glm::vec4{0.0f, 1.0f, 0.0f, 1.0f});
    quad.bindVertexArray(0);
    quad.draw();
    quad.primitive(0).vao->unbind();
}

void App::renderTerrainInstances()
{
    static std::list<std::shared_ptr<lix::Node>> nodeList;
    if(nodeList.empty())
    {
        for(size_t i{0}; i < 100; ++i)
        {
            auto node = std::make_shared<lix::Node>();
            node->setTranslation(glm::vec3{(rand() % 10000) * 0.02f - 100.0f,
                0.0f,
                (rand() % 10000) * 0.02f - 100.0f
            });
            nodeList.emplace_back(node);
        }
    }
    static lix::InstancedRendering<std::list<lix::NodePtr>> bushRendering{
        std::shared_ptr<lix::Mesh>(bushMesh->clone()), nodeList
    };

    instShader->bind();
    instShader->setUniform("u_time", time());
    instShader->setUniform("u_lights", lights);
    bushRendering.render(*instShader);
}

void App::renderBoneInstances()
{
    /*static auto nodeList = mooseNode->find("Root")->listNodes(); TODO: FIXME
    static lix::InstancedNodeRendering instancedRendering {
        std::shared_ptr<lix::Mesh>(boneMesh->clone()), nodeList
    };
    instancedRendering.refresh();
    instShader->bind();
    instancedRendering.render(*instShader);*/
}

void App::renderSkinnedModels()
{
    animShader->bind();
    animShader->setUniform("u_time", time());
    animShader->setUniform("u_lights", lights);
    for(auto node : {mooseNode, animatedCubeNode})
    {
        lix::renderSkinAnimationNode(*animShader, *node);
    }
    animShader->unbind();
}

void App::renderText()
{
    static lix::Text::Properties props{
        lix::Text::PropBuilder()
            .setAlignment(lix::Text::CENTER)
            .setTextScale(2.0f)
            .setLetterSpacing(1.0f)
            .setLineSpacing(1.0f)
            .setTextColor(lix::Color{1.0f, 1.0f, 0.5f, 1.0f})
            .setBorderColor(lix::Color::red)
    };

    static std::vector<std::shared_ptr<lix::Text>> texts = {
            std::shared_ptr<lix::Text>{new lix::Text(
                arialFont,
                lix::Text::PropBuilder()
                    .setTextColor(lix::Color::green)
                    .setTextScale(2.0f)
                    .setAlignment(lix::Text::Alignment::CENTER),
                "Selling full addy (g) 300M"
            )}
    };
    static lix::TextRendering textRendering {
        glm::vec2{SCREEN_WIDTH, SCREEN_HEIGHT},
        texts
    };
    texts.front()
        ->setTranslation(glm::vec3{-300.0f, -200.0f, 0.0f})
        ->setScale(glm::vec3{0.5f});
    textRendering.render();

    static auto borderTextShader = std::make_shared<lix::ShaderProgram>(
        assets::shaders::text_vert,
        assets::shaders::text_border_frag
    );
    static std::vector<lix::TextPtr> borderTexts = {std::make_shared<lix::Text>(arialFont,
        props,
        "- Main menu -\n- Options -\n- Ex*t -")};
    static lix::TextRendering borderTextRendering {
        glm::vec2{SCREEN_WIDTH, SCREEN_HEIGHT},
        borderTexts,
        borderTextShader
    };
    borderTextRendering.text(0)
        ->setTranslation(glm::vec3{300.0f, -200.0f, 0.0f})
        ->setScale(glm::vec3{0.5f});
    borderTextRendering.shaderProgram()
        ->bind()
        ->setUniform("u_border_color", borderTextRendering.text(0)->properties().borderColor)
        ->setUniform("u_border_width", 0.2f)
        ->setUniform("u_edge_smoothness", 0.05f);
    borderTextRendering.render();

    static auto wavyTextShader = std::make_shared<lix::ShaderProgram>(
        assets::shaders::text_wavy_vert,
        assets::shaders::text_frag
    );
    static std::vector<lix::TextPtr> wavyTexts = {std::make_shared<lix::Text>(arialFont,
        lix::Text::PropBuilder()
            .setTextColor(lix::Color::magenta)
            .setTextScale(2.0f)
            .setAlignment(lix::Text::Alignment::CENTER),
        "Drop party~~~ Rune 2h + more.")};
    static lix::TextRendering wavyTextRendering {
        glm::vec2{SCREEN_WIDTH, SCREEN_HEIGHT},
        wavyTexts,
        wavyTextShader
    };
    wavyTextRendering.text(0)
        ->setTranslation(glm::vec3{300.0f, 200.0f, 0.0f})
        ->setScale(glm::vec3{0.5f});
    wavyTextRendering.shaderProgram()->bind();
    wavyTextRendering.shaderProgram()->setUniform("u_time", time());
    wavyTextRendering.render();
}

void App::renderPolygons()
{
    static std::shared_ptr<lix::Material> polygonMat = lix::Material::Basic({0.5f, 0.5f, 1.0f});
    static lix::PolygonRendering rectangleRendering{rectangle->transformedPoints(),
    {
        std::make_shared<lix::TRS>(glm::vec3{0.0f, 0.0f, 0.0f}, glm::quat{1.0f, 0.0f, 0.0f, 0.0f}, glm::vec3{1.0f})
    }};
    static lix::PolygonRendering circleRendering{circle->points(),
    {
        circle,
        circle2
    }};
    shaderProgram->bind();
    lix::bindMaterial(*shaderProgram, *polygonMat);
    rectangleRendering.render(shaderProgram);
    circleRendering.render(shaderProgram);
}

void App::draw()
{
    postFBO->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cameraUBO->bufferSubData(sizeof(glm::mat4), sizeof(glm::mat4) + sizeof(glm::vec3)); // view

    renderModels();
    renderPolygons();
    renderTest();
    renderTerrainInstances();
    renderSkinnedModels();
    glClear(GL_DEPTH_BUFFER_BIT);
    renderBoneInstances();
    postFBO->unbind();
    static lix::BlurPass customBlurPass{glm::ivec2{SCREEN_WIDTH, SCREEN_HEIGHT},
        customBlurShader};
    customBlurPass.blur(postFBO->texture(GL_COLOR_ATTACHMENT1));
    static lix::BlurPass blurPass{glm::ivec2{SCREEN_WIDTH, SCREEN_HEIGHT}};
    blurPass.blur(customBlurPass.outputTexture());

    postFBO->bind();
    postFBO->blit(blitFBO, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0);
    float fp[4];
    postFBO->readPixel(32, 32, GL_RGBA, GL_FLOAT, fp);
    glm::vec4 v = postFBO->readPixel(32, 32);
    assert(v.x >= 0);
    postFBO->unbind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    blitFBO->bindTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE0);
    blurPass.outputTexture()->bind(GL_TEXTURE1);
    screenShader->bind();
    lix::renderScreen();
    glClear(GL_DEPTH_BUFFER_BIT);
    renderText();
}

void App::onKeyDown(lix::KeySym /*key*/, lix::KeyMod /*mod*/)
{

}

void App::onKeyUp(lix::KeySym /*key*/, lix::KeyMod /*mod*/)
{
    
}

void App::onMouseDown(lix::KeySym /*key*/, lix::KeyMod /*mod*/)
{
    
}

void App::onMouseUp(lix::KeySym /*key*/, lix::KeyMod /*mod*/)
{
    
}