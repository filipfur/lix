#include "glapplication.h"
#include "gleditor.h"
#include <fstream>

#include "glrendering.h"
#include "glshaderprogram.h"
#include "gltfloader.h"
#include "gluniformbuffer.h"
#include "json.h"

#include "aabb.h"
#include "charactercontroller.h"
#include "collision.h"
#include "convexhull.h"
#include "glarrow.h"
#include "glconsole.h"
#include "glgeometry.h"
#include "gltextrendering.h"
#include "inertia.h"
#include "infinite_iterator.h"
#include "polygon.h"
#include "primer.h"
#include "rigidbody.h"
#include "sphere.h"

#include "gen/fonts/josefin_sans.h"
#include "gen/levels/home.h"
#include "gen/levels/level.h"
#include "gen/objects/cube.h"
#include "gen/objects/donut.h"
#include "gen/objects/lilbro.h"
#include "gen/objects/macman.h"
#include "gen/objects/platform.h"
#include "gen/objects/rat.h"
#include "gen/objects/spyro_ps1.h"

#define LIX_LORES
#ifdef LIX_LORES
static constexpr float WINDOW_X = 1080;
static constexpr float WINDOW_Y = 720;
#else
static constexpr float WINDOW_X = 1920;
static constexpr float WINDOW_Y = 1080;
#endif

static constexpr float WINDOW_REL_X = WINDOW_X / 1920.0f;
static constexpr float WINDOW_REL_Y = WINDOW_Y / 1080.0f;

static constexpr float NEAR = 0.01;
static constexpr float FAR = 100;
static constexpr float CAMERA_DISTANCE = 12.0f;
static const glm::mat4 PERSPECTIVE{
    glm::perspective(45.0f, WINDOW_X / WINDOW_Y, NEAR, FAR)};
static const glm::vec2 RESOLUTION{WINDOW_X, WINDOW_Y};

std::shared_ptr<lix::VAO> planeHUD() {
    static auto vao =
        std::make_shared<lix::VAO>(lix::Attributes{lix::Attribute::VEC4},
                                   std::vector<GLfloat>{
                                       -0.5f,
                                       -0.5f,
                                       0.0f,
                                       0.0f,
                                       0.5f,
                                       -0.5f,
                                       1.0f,
                                       0.0f,
                                       -0.5f,
                                       0.5f,
                                       0.0f,
                                       1.0f,
                                       0.5f,
                                       0.5f,
                                       1.0f,
                                       1.0f,
                                   },
                                   std::vector<GLuint>{0, 1, 2, 2, 1, 3});
    return vao;
}

const char *vertexHUD = LIX_SHADER_VERSION R"(
layout (location = 0) in vec4 aVertex;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

out vec2 texCoords;
out vec2 position;
//out vec2 scale;

uniform float u_time;

void main()
{
    position = vec2(aVertex.x, -aVertex.y);
    texCoords = aVertex.zw;
    /*mat4 m = u_model;
    scale = vec2(
        sqrt(m[0][0] * m[0][0] + m[0][1] * m[0][1] + m[0][2] * m[0][2]),
        sqrt(m[1][0] * m[1][0] + m[1][1] * m[1][1] + m[1][2] * m[1][2])
    );*/
    vec3 p0 = vec3(u_model * vec4(position, 0.0, 1.0));
    gl_Position = u_projection * u_view * vec4(p0, 1.0);
}
)";

const char *fragmentHUD = LIX_SHADER_VERSION R"(
precision highp float;

out vec4 FragColor;
in vec2 texCoords;
in vec2 position;
in vec2 scale;

uniform vec4 u_base_color;

void main()
{
    //position.x *= 1;
    //vec2 p = position * scale;

    //float a = 1.0 - smoothstep(550, 552, max(length(p), 0.0));
    //a *= 0.8 - min(1.0, max(0.0, smoothstep(440, 442, p.x + -p.y) - smoothstep(460, 462, p.x + -p.y) + smoothstep(480, 482, p.x + -p.y))) * 0.4;
    //FragColor = vec4(u_base_color.rgb, u_base_color.a * a);
    FragColor = vec4(1,0,0,1);
}
)";

const char *vertexSource = LIX_SHADER_VERSION R"(
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexCoords;

    layout (std140) uniform CameraBlock
    {
        mat4 u_projection;
        mat4 u_view;
        vec3 u_eye_pos;
        float padding;
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

const char *vertexSkinSource = LIX_SHADER_VERSION R"(
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
        float padding;
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

const char *fragmentSource = LIX_SHADER_VERSION R"(
    precision highp float;

    layout (std140) uniform LightBlock
    {
        vec4 u_light_color;
        vec4 u_shadow_color;
        vec4 u_shine_color;
    };

    in vec2 texCoords;
    in vec3 normal;

    out vec4 fragColor;

    uniform sampler2D u_texture;
    uniform vec4 u_base_color;

    const vec3 lightDir = vec3(0.0, -1.0, 0.0);

    void main()
    {
        vec4 tex = texture(u_texture, texCoords);
        fragColor = u_base_color * tex * u_light_color;
        fragColor.rgb *= u_light_color.rgb;
        float emission = dot(-lightDir, normal);
        //fragColor.rgb = vec3(0.0, 0.0, 0.0);
        fragColor.rgb = mix(fragColor.rgb, fragColor.rgb + u_shine_color.rgb, step(0.5, emission) * 0.2);
        //fragColor.rgb = mix(fragColor.rgb, fragColor.rgb * 0.5 * u_shadow_color.rgb, step(0.5, -emission) * 0.2);
        fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2));
    }
)";

const char *skyboxVertSource = LIX_SHADER_VERSION R"(
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
        mat4 view = mat4(mat3(u_view));
        vec4 pos = u_projection * view * vec4(position, 1.0);
        gl_Position = pos.xyww;
    }
)";

const char *skyboxFragSource = LIX_SHADER_VERSION R"(
precision highp float;
// Simplex 2D noise
//
vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v){
  const vec4 C = vec4(0.211324865405187, 0.366025403784439,
           -0.577350269189626, 0.024390243902439);
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = mod(i, 289.0);
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
  + i.x + vec3(0.0, i1.x, 1.0 ));
  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
    dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;
  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

    in vec2 texCoords;
    in vec3 normal;

    out vec4 fragColor;

    uniform float u_time;

    uniform vec3 u_color_lo;
    uniform vec3 u_color_hi;

    const vec3 lightDir = vec3(0.0, -1.0, 0.0);

    void main()
    {
        fragColor = vec4(mix(u_color_lo, u_color_hi, texCoords.y), 1.0);
        fragColor.rgb += max(0.0, snoise(vec2(sin(texCoords.x * 2.0 * 3.141592) * 0.5 + 0.5 + u_time * 0.02, texCoords.y) * vec2(2,2))) * 0.5;
    }
)";

static void dumpPolygon(const std::filesystem::path &path,
                        const lix::Polygon &polygon,
                        const std::vector<lix::TRS *> &frames) {
    std::ofstream ofs(path);
    if (ofs) {
        json::Json jo{json::Object};
        json::Json pointsObj{json::Array};
        for (const auto &p : polygon.points()) {
            json::Json arr{json::Array};
            arr.put(p.x);
            arr.put(p.y);
            arr.put(p.z);
            pointsObj.insert(arr);
        }
        jo.add("points", pointsObj);
        json::Json framesObj{json::Array};
        for (const auto &trs : frames) {
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

template <typename T> void dumpBytes(std::ofstream &ofs, const T &t) {
    char *ptr = (char *)&t;
    for (size_t i{0}; i < sizeof(T); ++i) {
        ofs << ptr[i];
    }
}

void dumpBytes(std::ofstream &ofs, char *ptr, size_t len) {
    for (size_t i{0}; i < len; ++i) {
        ofs << ptr[i];
    }
}

void dumpBinary(const std::filesystem::path &path, const lix::Polygon &polygon,
                const std::vector<lix::TRS *> &frames) {
    std::ofstream ofs(path, std::ios::binary);
    if (ofs) {
        const char *cp = (char *)polygon.points().data();
        const size_t n = polygon.points().size();
        const size_t numBytes = n * sizeof(glm::vec3);
        dumpBytes(ofs, n);
        dumpBytes(ofs, (char *)polygon.points().data(), numBytes);
        const size_t n2 = frames.size();
        dumpBytes(ofs, n2);
        for (lix::TRS *trs : frames) {
            dumpBytes(ofs, trs->translation());
            dumpBytes(ofs, trs->rotation());
            dumpBytes(ofs, trs->scale());
        }
        ofs << (unsigned char)0xAB;
    }
    ofs.close();
}

struct Sky {
    lix::Color light;
    lix::Color shadow;
    lix::Color shine;
    lix::Color low;
    lix::Color high;
};

struct Entity {
    std::shared_ptr<lix::Node> node;
    lix::RigidBody *rigidBody;
    std::shared_ptr<lix::CharacterController> characterController;
};

struct Consumable {
    std::shared_ptr<lix::Node> node;
    std::shared_ptr<lix::Shape> shape;
    const glm::vec3 origo;
    glm::vec3 location;
    bool consumed;
    lix::TRS *travelTo;
    lix::Timer fadeOut;
};

struct Platform {
    lix::ConvexHull convexHull;
    std::shared_ptr<lix::StaticBody> rigidBody;
};

class App : public lix::Application, public lix::Editor {
  public:
    App()
        : lix::Application{WINDOW_X, WINDOW_Y, "example_platformer"},
          lix::Editor{PERSPECTIVE, RESOLUTION, CAMERA_DISTANCE} {}

    virtual void init() override;
    virtual void tick(float dt) override;
    virtual void draw() override;

    virtual bool onKeyDown(lix::KeySym key, lix::KeyMod mod) override;
    virtual bool onKeyUp(lix::KeySym key, lix::KeyMod mod) override;
    virtual bool onMouseMove(float x, float y, float xrel, float yrel) override;

    virtual void onSubjectTransformed(lix::Node *subject,
                                      Transformation transformation) override;

    lix::NodePtr createNode(const gltf::Mesh &gltfMesh,
                            const glm::vec3 &position,
                            const glm::vec3 &rotation, const glm::vec3 &scale) {
        auto node = std::make_shared<lix::Node>(
            position,
            glm::dot(rotation, rotation) > FLT_EPSILON
                ? glm::angleAxis(glm::radians(rotation.x),
                                 glm::vec3{1.0f, 0.0f, 0.0f}) *
                      glm::angleAxis(glm::radians(rotation.y),
                                     glm::vec3{0.0f, 1.0f, 0.0f}) *
                      glm::angleAxis(glm::radians(rotation.z),
                                     glm::vec3{0.0f, 0.0f, 1.0f})
                : glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
            scale);
        auto mesh = gltf::loadMesh(gltfMesh);
        node->setMesh(mesh);
        return node;
    }

    Entity &emplaceEntity(std::shared_ptr<lix::Node> node,
                          lix::RigidBody *rigidBody) {
        entities.push_back({node, rigidBody});
        return entities.back();
    }

    Consumable &emplaceConsumable(std::shared_ptr<lix::Node> node,
                                  std::shared_ptr<lix::Shape> shape) {
        consumables.push_back({node, shape, node->translation(),
                               node->translation(), false, nullptr});
        return consumables.back();
    }

    std::shared_ptr<lix::CharacterController>
    emplaceCharacterController(std::shared_ptr<lix::Shape> shape) {
        return characterControllers.emplace_back(
            new lix::CharacterController(std::make_shared<lix::DynamicBody>(
                shape, 1.0f, lix::cubeInertiaTensor(1.0f, 1.0f))));
    }

    std::shared_ptr<lix::StaticBody>
    emplaceStaticBody(std::shared_ptr<lix::Shape> shape) {
        return staticBodies.emplace_back(new lix::StaticBody(shape));
    }

    lix::NodePtr emplaceHUDElement(const glm::vec2 &pos,
                                   const glm::vec2 &size) {
        auto mesh = std::make_shared<lix::Mesh>(
            planeHUD(), std::make_shared<lix::Material>(lix::Color::white));
        auto node = hudObjects.emplace_back(new lix::Node(
            glm::vec3{pos, 0.0f}, glm::quat{1.0f, 0.0f, 0.0f, 0.0f},
            glm::vec3{size, 1.0f}));
        node->setMesh(mesh);
        return node;
    }

    std::shared_ptr<lix::Sphere>
    createSphereCollider(lix::Node &node, const gltf::Mesh &gltfMesh,
                         std::optional<float> r = std::nullopt) {
        std::vector<glm::vec3> vertices;
        std::vector<unsigned int> indices;
        gltf::loadAttributes(gltfMesh, 0, gltf::A_POSITION, vertices, indices);

        glm::mat4 m = glm::mat4(glm::mat3(node.modelMatrix()));
        for (auto &v : vertices) {
            v = glm::vec3(m * glm::vec4{v, 1.0f});
        }
        auto [min, max] = lix::extremePoints(vertices);

        float radii{0.0f};
        if (r.has_value()) {
            radii = r.value();
        } else {
            glm::vec3 extent = max - min;
            radii = extent.y * 0.5f;
        }
        printf("radii=%.2f\n", radii);

        auto sphere = std::make_shared<lix::Sphere>(&node, radii);

        auto aabb = std::make_shared<lix::AABB>(&node, min, max);
        sphere->setSimplified(aabb);

        return sphere;
    }

    std::shared_ptr<lix::Polygon>
    createMeshCollider(lix::Node &node, const gltf::Mesh &gltfMesh) {
        auto poly = gltf::loadMeshCollider(gltfMesh, true);
        poly->setTRS(&node);

        printf("createMeshCollider: %s #%zu\n", gltfMesh.name.c_str(),
               poly->points().size());

        auto aabb = std::make_shared<lix::AABB>(poly->trs(), poly.get());
        poly->setSimplified(aabb);
        return poly;
    }

    std::shared_ptr<Platform>
    getPlatform(std::shared_ptr<lix::StaticBody> staticBody) {
        std::shared_ptr<Platform> rval{nullptr};
        for (auto &platform : platforms) {
            if (platform && platform->rigidBody.get() == staticBody.get()) {
                rval = platform;
                break;
            }
        }
        return rval;
    }

    std::shared_ptr<lix::Node>
    loadCharacter(const gltf::Node &gltfNode, const gltf::Skin &gltfSkin,
                  const gltf::Mesh &gltfMesh,
                  std::vector<const gltf::Animation *> gltfAnimations,
                  std::optional<float> radii = std::nullopt) {
        auto node = gltf::loadNode(gltfNode);
        gltf::loadSkin(node.get(), gltfSkin);
        for (const auto &gltfAnimation : gltfAnimations) {
            gltf::loadAnimation(node.get(), *gltfAnimation);
        }
        auto cctrl =
            emplaceCharacterController(createSphereCollider(*node, gltfMesh));
        auto &ent = emplaceEntity(node, cctrl->dynamicBody.get());
        node->skin()->setCurrentAnimation(node->skin()->animations().begin());
        ent.characterController = cctrl;
        skinnedNodes.push_back(node);
        return node;
    }

    lix::ShaderProgramPtr objectShader;
    lix::ShaderProgramPtr skinnedShader;
    lix::ShaderProgramPtr skyboxShader;
    lix::ShaderProgramPtr hudShader;
    std::vector<lix::NodePtr> objectNodes;
    std::vector<lix::NodePtr> skinnedNodes;
    std::vector<lix::NodePtr> hudObjects;

    lix::NodePtr sphereNode;

    std::vector<Entity> entities;
    std::vector<Consumable> consumables;
    std::vector<std::shared_ptr<lix::CharacterController>> characterControllers;
    std::array<std::shared_ptr<Platform>, 3> platforms;
    std::vector<std::shared_ptr<lix::StaticBody>> staticBodies;
    std::shared_ptr<lix::VAO> closestFaceVAO;
    std::shared_ptr<lix::VAO> platformConvexVAO;
    std::vector<std::shared_ptr<lix::Node>> gimbalArrows;
    std::vector<std::shared_ptr<lix::Node>> normalArrows;
    bool xrayMode{false};
    std::unique_ptr<lix::TextRendering> textRendering;
    std::vector<lix::TextPtr> texts;
    bool showDialogue{false};
    std::vector<std::pair<glm::vec3, std::function<void()>>> interactionPoints;
    float cameraHeight{3.0f};
    std::array<Sky, 4> skies = {
        Sky{{0xffffff},
            {0x000033},
            {0x991900},
            lix::Color(0.5f, 0.95f, 1.0f),
            lix::Color(0.0f, 0.3f, 0.8f)},
        Sky{{0xffcccc}, {0x000033}, {0x991900}, {0x000000}, {0xffffff}},
        Sky{{0xffcccc}, {0x000033}, {0x991900}, {0xff6b75}, {0x004ccc}},
        Sky{{0xffcccc}, {0x000033}, {0x991900}, {0xDFFF00}, {0xffffff}}};
    // std::array<Sky, 1>::const_iterator currentSky = skies.begin();
    infinite_iterator<std::array<Sky, 4>> currentSky{skies, skies.begin()};
    struct LightBlock {
        glm::vec4 lightColor;
        glm::vec4 shadowColor;
        glm::vec4 shineColor;
    } lightBlock;
    std::shared_ptr<lix::UniformBuffer> lightUBO;
    std::shared_ptr<lix::Console> console;
};

int main(int argc, char *argv[]) {
    App app;
    app.run();
    return 0;
}

void printDuration(const lix::SkinAnimation &skinAnimation) {
    printf("%s: %.1fs\n", skinAnimation.name().c_str(),
           (skinAnimation.end() - skinAnimation.start()));
}

void App::init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    setInputAdapter(this);

    setOnKeyDown(SDLK_RETURN, [this](auto, auto) {
        for (const auto &p : interactionPoints) {
            glm::vec3 d = p.first - entities.front().node->translation();
            if (glm::dot(d, d) < 2.0f) {
                p.second();
            }
        }
    });

    static auto font =
        std::make_shared<lix::Font>(assets::fonts::josefin_sans::create());
    auto text = texts.emplace_back(new lix::Text(
        font, lix::Text::PropBuilder().setTextColor(0x333344), ""));
    text->setTranslation(glm::vec3{-WINDOW_X * 0.4f * WINDOW_REL_X,
                                   -WINDOW_Y * 0.29f * WINDOW_REL_Y, 0.0f});
    text->setScale(glm::vec3{WINDOW_REL_X, WINDOW_REL_Y, 1.0f});

    console = std::make_shared<lix::Console>(
        "cmd>",
        texts.emplace_back(new lix::Text(
            font, lix::Text::PropBuilder().setTextColor(0x333344), "")));
    console->textNode->setTranslation(glm::vec3{-WINDOW_X * 0.4f * WINDOW_REL_X,
                                                -WINDOW_Y * 0.4f * WINDOW_REL_Y,
                                                0.0f});
    console->textNode->setVisible(false);

    console->commands.emplace_back(new lix::custom_command(
        "print.colors", [](lix::Console &console, const std::string &cmd) {
            for (const auto &c : console.commands) {
                if (auto colorcmd =
                        dynamic_cast<lix::color_command *>(c.get())) {
                    printf("%s=%s\n", colorcmd->shortname,
                           colorcmd->color().hexString().c_str());
                }
            }
            return true;
        }));
    console->commands.emplace_back(new lix::custom_command(
        "print.nodes", [this](lix::Console &console, const std::string &cmd) {
            for (const auto &o : objectNodes) {
                auto p = o->translation();
                printf("'%s': [%.2f %.2f %.2f]\n", o->name().c_str(), p.x, p.y,
                       p.z);
            }
            for (const auto &o : skinnedNodes) {
                auto p = o->translation();
                printf("'%s': [%.2f %.2f %.2f]\n", o->name().c_str(), p.x, p.y,
                       p.z);
            }
            return true;
        }));

    console->commands.emplace_back(new lix::color_command(
        "sky.light", [this]() -> lix::Color & { return currentSky->light; }));
    console->commands.emplace_back(new lix::color_command(
        "sky.shadow", [this]() -> lix::Color & { return currentSky->shadow; }));
    console->commands.emplace_back(new lix::color_command(
        "sky.shine", [this]() -> lix::Color & { return currentSky->shine; }));
    console->commands.emplace_back(new lix::color_command(
        "sky.high", [this]() -> lix::Color & { return currentSky->high; }));
    console->commands.emplace_back(new lix::color_command(
        "sky.low", [this]() -> lix::Color & { return currentSky->low; }));
    console->commands.emplace_back(new lix::iterate_command("sky", currentSky));

    console->commands.emplace_back(new lix::custom_command(
        "tele", [this](lix::Console &console, const std::string &cmd) {
            auto teleTo = cmd.substr(cmd.find('=') + 1);
            printf("teleTo=%s\n", teleTo.c_str());
            for (const auto &o : objectNodes) {
                if (o->name() == teleTo) {
                    entities.front().node->setTranslation(o->translation());
                    return true;
                }
            }
            for (const auto &o : skinnedNodes) {
                if (o->name() == teleTo) {
                    entities.front().node->setTranslation(o->translation());
                    return true;
                }
            }
            return false;
        }));

    textRendering.reset(
        new lix::TextRendering(glm::vec2{WINDOW_X, WINDOW_Y}, texts));

    objectShader.reset(new lix::ShaderProgram(vertexSource, fragmentSource));
    skinnedShader.reset(
        new lix::ShaderProgram(vertexSkinSource, fragmentSource));
    skyboxShader.reset(
        new lix::ShaderProgram(skyboxVertSource, skyboxFragSource));
    camera().setupUBO(
        {objectShader.get(), skinnedShader.get(), skyboxShader.get()});
    lightUBO.reset(new lix::UniformBuffer(sizeof(LightBlock),
                                          (void *)&lightBlock, "LightBlock", 2,
                                          GL_STATIC_DRAW));
    lightUBO->bindShaders({objectShader.get(), skinnedShader.get()});

    hudShader.reset(new lix::ShaderProgram(vertexHUD, fragmentHUD));
    hudShader->bind();
    hudShader->setUniform("u_projection", textRendering->projection());
    hudShader->setUniform("u_view", textRendering->view());

    emplaceHUDElement({0.0f, -WINDOW_Y * WINDOW_REL_Y * 0.3f},
                      {1100 * WINDOW_REL_X, 120 * WINDOW_REL_Y})
        ->mesh()
        ->material()
        ->setBaseColor(0xefefe0);

    gimbalArrows.insert(
        gimbalArrows.end(),
        {lix::arrow(lix::Color::red, glm::vec3{0.0f, 0.0f, 0.0f},
                    glm::vec3{1.0f, 0.0f, 0.0f}),
         lix::arrow(lix::Color::green, glm::vec3{0.0f, 0.0f, 0.0f},
                    glm::vec3{0.0f, 1.0f, 0.0f}),
         lix::arrow(lix::Color::blue, glm::vec3{0.0f, 0.0f, 0.0f},
                    glm::vec3{0.0f, 0.0f, 1.0f})});

    auto [vs, is] = lix::sphere(32, 16);
    sphereNode.reset(new lix::Node());
    sphereNode->setScale(glm::vec3{1.0f});
    sphereNode->setMesh(std::make_shared<lix::Mesh>(
        std::make_shared<lix::VAO>(
            lix::Attributes{lix::VEC3, lix::VEC3, lix::VEC2}, vs, is),
        std::make_shared<lix::Material>(lix::Color::white)));

    auto playerNode =
        loadCharacter(assets::objects::macman::Macman_node,
                      assets::objects::macman::Macman_skin,
                      assets::objects::macman::Cube_001_mesh,
                      {&assets::objects::macman::Idle_anim,
                       &assets::objects::macman::Running_anim,
                       &assets::objects::macman::Jump_anim,
                       &assets::objects::macman::Leaping_anim,
                       &assets::objects::macman::StrafingLeft_anim,
                       &assets::objects::macman::StrafingRight_anim});
    playerNode->setTranslation(glm::vec3{0.2f, 6.0f, 0.2f});

    for (const auto &obj : assets::levels::level::nodes) {
        if (obj.name == "Donut") {
            auto node = gltf::loadNode(obj);
            auto sphere = createSphereCollider(*node, *obj.mesh, 0.7f);
            sphere->radii();
            emplaceConsumable(node, sphere);
            objectNodes.push_back(node);
        } else if (obj.mesh->name == "Rat") {
            auto node = loadCharacter(assets::objects::rat::Rat_node,
                                      assets::objects::rat::Rat_skin,
                                      assets::objects::rat::Rat_mesh,
                                      {
                                          &assets::objects::rat::Idle_anim,
                                          &assets::objects::rat::Bounce_anim,
                                          &assets::objects::rat::Jump_anim,
                                      });
            node->skin()->animations()["Bounce"]->setLooping(false);
            node->skin()->animations()["Jump"]->setLooping(false);

            node->setTranslation(obj.translation);
            node->setRotation(obj.rotation);
            node->setScale(obj.scale);

            characterControllers.back()->setMaxJumpSpeed(2.0f);

            const glm::quat r0 = obj.rotation;
            interactionPoints.emplace_back(
                node->translation(), [this, r0, node]() {
                    if (showDialogue) {
                        showDialogue = false;
                        texts.front()->setText("");
                        node->setRotation(r0);
                    } else {
                        glm::vec3 d = entities.front().node->translation() -
                                      node->translation();
                        node->setRotation(glm::angleAxis(
                            glm::atan(d.x, d.z), glm::vec3{0.0f, 1.0f, 0.0f}));
                        texts.front()->setText(
                            "Disaster!!! I've lost all my children.\nWill you "
                            "help me find them?");
                        showDialogue = true;
                    }
                });
        } else if (obj.mesh->name == "InfinityBridge") {
            auto node = gltf::loadNode(obj);
            objectNodes.push_back(node);
        } else {
            auto node = gltf::loadNode(obj);
            emplaceStaticBody(createMeshCollider(*node, *obj.mesh));
            objectNodes.push_back(node);
        }
    }
    auto lilbroNode = loadCharacter(assets::objects::lilbro::Lilbro_node,
                                    assets::objects::lilbro::Lilbro_skin,
                                    assets::objects::lilbro::Cube_004_mesh,
                                    {&assets::objects::lilbro::Idle_anim});
}

void App::tick(float dt) {
    float t = lix::Time::seconds();

    lightBlock.lightColor = currentSky->light;
    lightBlock.shadowColor = currentSky->low;
    lightBlock.shineColor = currentSky->high;
    lightUBO->bufferData();

    for (const auto &entity : entities) {
        if (entity.node->name() == "Rat") {
            if (showDialogue) {
                entity.characterController->stopJump();
                entity.node->skin()->setCurrentAnimation(
                    entity.node->skin()->animations().find("Idle"));
            } else if (entity.characterController->movementState ==
                       lix::CharacterController::MovementState::IDLE) {
                entity.characterController->jump();
                entity.node->skin()->setCurrentAnimation(
                    entity.node->skin()->animations().find("Bounce"));
                auto &anim = entity.node->skin()->currentAnimation()->second;
                anim->setTime(anim->start());
            }
        }
    }

    for (auto &consumable : consumables) {
        if (consumable.fadeOut.active()) {
            if (consumable.fadeOut.elapsed()) {
                objectNodes.erase(
                    std::remove_if(objectNodes.begin(), objectNodes.end(),
                                   [&consumable](const auto &o) {
                                       return o.get() == consumable.node.get();
                                   }),
                    objectNodes.end());
                continue;
            } else {
                float f = sinf(consumable.fadeOut.progress() * 1.0f *
                               glm::pi<float>());
                consumable.location.y += 4.0f * f * dt;
                consumable.node->setScale(consumable.node->scale() +
                                          glm::vec3{f * dt});
            }
        } else if (consumable.travelTo) {
            consumable.location =
                glm::mix(consumable.location,
                         consumable.travelTo->translation(), 16.0f * dt);
            glm::vec3 delta =
                consumable.travelTo->translation() - consumable.location;
            if (glm::dot(delta, delta) < 0.05f) {
                consumable.consumed = true;
                consumable.fadeOut.set(lix::Time::fromSeconds(0.5f));
            }
        }
        consumable.node->setTranslation(
            {consumable.location.x,
             consumable.location.y + sinf(t * 4.0f) * 0.1f,
             consumable.location.z});
        consumable.node->setRotation(
            glm::angleAxis(t * 1.0f, glm::vec3{0.0f, 1.0f, 0.0f}) *
            glm::angleAxis(sinf(t * 0.7f) * 0.3f, glm::vec3{1.0f, 0.0f, 0.0f}));
    }

    static lix::Timer timerSeconds{lix::Time::fromSeconds(0.2f)};
    if (!timerSeconds.elapsed()) {
        return;
    }

    for (const auto &skinned : skinnedNodes) {
        skinned->skin()->currentAnimation()->second->update(dt);
    }

    auto &playerCtrl = characterControllers.front();
    const glm::vec3 playerPos =
        playerCtrl->dynamicBody->shape->trs()->translation();
    camera().setTarget(playerPos + glm::vec3{0.0f, 1.0f, 0.0f});
    camera().setTranslation(
        playerPos - glm::vec3{sinf(playerCtrl->yaw) * 6.0f, -cameraHeight,
                              cosf(playerCtrl->yaw) * 6.0f});
    camera().refresh(dt);

    if (playerPos.y < -10.0f) // respawn
    {
        playerCtrl->dynamicBody->shape->trs()->setTranslation(
            glm::vec3{0.0f, 6.0f, 0.0f});
        playerCtrl->dynamicBody->velocity = glm::vec3{0.0f};
    }

    static lix::Collision collision;
    for (auto &cctrl : characterControllers) {
        auto dyn = cctrl->dynamicBody.get();
        if (cctrl->deltaControl.y > 0) {
            dyn->shape->trs()->applyTranslation(glm::vec3{0.0f, 0.05f, 0.0f});
            dyn->velocity.y = cctrl->maxJumpSpeed;
            cctrl->deltaControl.y = 0.0f;
            cctrl->platform.reset();
        }
        if (cctrl->platform) {
            auto &v = cctrl->dynamicBody->velocity;
            if (v.x * v.x + v.z * v.z > 0.01f) {
                static std::shared_ptr<Platform> prevPlatform{nullptr};
                auto platform = getPlatform(cctrl->platform);
                const glm::vec3 p0 =
                    cctrl->dynamicBody->shape->trs()->translation();
                float radii =
                    dynamic_cast<lix::Sphere *>(cctrl->dynamicBody->shape.get())
                        ->radii();
                auto intersectionPoint = platform->convexHull.rayIntersect(
                    p0, glm::vec3{0.0f, -1.0f, 0.0f});
                float dist = (intersectionPoint->y + radii) - p0.y;
                if (intersectionPoint.has_value() && dist < 0.1f &&
                    dist > -0.1f) {
                    dyn->shape->trs()->setTranslation(
                        {p0.x, intersectionPoint->y + radii, p0.z});
                    cctrl->floorCrumbling = 0;
                } else if (cctrl->floorCrumbling > 9) {
                    cctrl->floorCrumbling = 0;
                    cctrl->platform.reset();
                } else {
                    ++cctrl->floorCrumbling;
                }

                dyn->shape->trs()->setRotation(glm::slerp(
                    dyn->shape->trs()->rotation(),
                    glm::angleAxis(cctrl->yaw, glm::vec3{0.0f, 1.0f, 0.0f}),
                    4.0f * dt));

                if (prevPlatform != platform) {
                    platformConvexVAO.reset(
                        new lix::VAO(lix::Attributes{lix::Attribute::VEC3},
                                     platform->convexHull.points()));
                    prevPlatform = platform;
                }
            }
        } else {
            // apply gravity
            dyn->velocity.y += -9.82f * dt;
            static lix::TRS lastTRS;
            for (auto &staticBody : staticBodies) {
                if (lix::collides(*dyn->shape, *staticBody->shape,
                                  &collision)) {
                    dyn->velocity.y = 0.0f;
                    dyn->shape->trs()->applyTranslation(
                        collision.normal * (collision.penetrationDepth));

                    cctrl->platform = staticBody;
                    bool loaded{false};
                    auto platform = getPlatform(staticBody);
                    if (platform == nullptr) {
                        printf("created platform convex\n");
                        for (size_t i{platforms.size() - 1}; i > 0; --i) {
                            std::swap(platforms[i], platforms[i - 1]);
                        }
                        auto pts = std::dynamic_pointer_cast<lix::Polygon>(
                                       staticBody->shape)
                                       ->transformedPoints();
                        printf("pts=%zu\n", pts.size());
                        auto unique = lix::uniqueVertices(pts);
                        platforms[0].reset(new Platform{unique, staticBody});
                        platform = platforms[0];
                        for (const auto &f : platform->convexHull) {
                            normalArrows.emplace_back(
                                lix::arrow(lix::Color::magenta,
                                           (f.half_edge->vertex +
                                            f.half_edge->next->vertex +
                                            f.half_edge->next->next->vertex) *
                                               0.3333f,
                                           f.normal));
                        }
                    }
                }
            }

            lastTRS.setTranslation(dyn->shape->trs()->translation());
            lastTRS.setRotation(dyn->shape->trs()->rotation());
            lastTRS.setScale(dyn->shape->trs()->scale());
        }
    }

    for (auto &cctrl : characterControllers) {
        auto dyn = cctrl->dynamicBody.get();
        glm::vec3 x0z = glm::normalize(
            glm::vec3{-cctrl->deltaControl.x, 0.0f, -cctrl->deltaControl.z});
        x0z = dyn->shape->trs()->rotation() * x0z;
        float acc = cctrl->onGround() ? 16.0f : 2.0f;
        float dacc = cctrl->onGround() ? 12.0f : 1.0f;
        if (glm::dot(x0z, x0z) > lix::EPSILON) {
            dyn->velocity.x = glm::mix(dyn->velocity.x,
                                       x0z.x * cctrl->maxWalkSpeed, acc * dt);
            dyn->velocity.z = glm::mix(dyn->velocity.z,
                                       x0z.z * cctrl->maxWalkSpeed, acc * dt);
        } else {
            if (dyn->velocity.x * dyn->velocity.x > 0.0001f) {
                dyn->velocity.x = glm::mix(dyn->velocity.x, 0.0f, dacc * dt);
            } else {
                dyn->velocity.x = 0.0f;
            }
            if (dyn->velocity.z * dyn->velocity.z > 0.0001f) {
                dyn->velocity.z = glm::mix(dyn->velocity.z, 0.0f, dacc * dt);
            } else {
                dyn->velocity.z = 0.0f;
            }
        }
        dyn->shape->trs()->applyTranslation(dyn->velocity * dt);
        cctrl->updateMovementState();

        for (auto &consumable : consumables) {
            if (consumable.travelTo) {
                continue;
            }
            if (dyn->shape->test(*consumable.shape)) {
                consumable.travelTo = dyn->shape->trs();
            }
        }
    }

    for (auto &obj : objectNodes) {
        if (obj->mesh()->name() == "InfinityBridge") {
            obj->applyTranslation(
                {0.0f, sinf(lix::Time::seconds() * 4.0f) * 0.2f * dt, 0.0f});
            obj->applyRotation(glm::angleAxis(glm::radians(dt * -15.0f),
                                              glm::vec3{1.0f, 0.0f, 0.0f}));
        }
    }

    for (auto &ent : entities) {
        if (ent.node->skin()) {
            if (ent.characterController) {
                switch (ent.characterController->movementState) {
                case lix::CharacterController::MovementState::IDLE:
                    ent.node->skin()->setCurrentAnimation(
                        ent.node->skin()->animations().find("Idle"));
                    break;
                case lix::CharacterController::MovementState::JUMPING:
                    ent.node->skin()->setCurrentAnimation(
                        ent.node->skin()->animations().find("Leaping"));
                    break;
                case lix::CharacterController::MovementState::FALLING:
                    ent.node->skin()->setCurrentAnimation(
                        ent.node->skin()->animations().find("Jump"));
                    break;
                case lix::CharacterController::MovementState::FORWARD:
                    ent.node->skin()->setCurrentAnimation(
                        ent.node->skin()->animations().find("Running"));
                    break;
                case lix::CharacterController::MovementState::LEFT:
                    ent.node->skin()->setCurrentAnimation(
                        ent.node->skin()->animations().find("StrafingLeft"));
                    break;
                case lix::CharacterController::MovementState::RIGHT:
                    ent.node->skin()->setCurrentAnimation(
                        ent.node->skin()->animations().find("StrafingRight"));
                    break;
                }
                break; // only for player for now this is HACK
            }
        }
    }
}

void App::draw() {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    objectShader->bind();
    if (xrayMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    for (const auto &obj : objectNodes) {
        lix::renderNode(*objectShader, *obj);
    }
    skinnedShader->bind();
    for (const auto &skinned : skinnedNodes) {
        lix::renderSkinAnimationNode(*skinnedShader, *skinned);
    }

    static auto tex = lix::Texture::Basic();
    tex->bind();

    skyboxShader->bind();
    skyboxShader->setUniform("u_time", lix::Time::seconds());
    skyboxShader->setUniform("u_color_lo", currentSky->low.vec3());
    skyboxShader->setUniform("u_color_hi", currentSky->high.vec3());
    glFrontFace(GL_CW);
    lix::renderNode(*skyboxShader, *sphereNode);
    glFrontFace(GL_CCW);

    objectShader->bind();
    glClear(GL_DEPTH_BUFFER_BIT);

    if (xrayMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_CULL_FACE);
        auto model = glm::mat4(1.0f);
        if (closestFaceVAO) {
            objectShader->setUniform("u_model", model);
            objectShader->setUniform("u_base_color", lix::Color::red.vec4());
            closestFaceVAO->bind();
            closestFaceVAO->draw();
        }
        if (platformConvexVAO) {
            objectShader->setUniform("u_model", model);
            objectShader->setUniform(
                "u_base_color", lix::Color::yellow * lix::Color::opacity(0.5f));
            platformConvexVAO->bind();
            platformConvexVAO->draw();
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            objectShader->setUniform("u_base_color", lix::Color::magenta);
            platformConvexVAO->bind();
            platformConvexVAO->draw();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        glEnable(GL_CULL_FACE);
        glClear(GL_DEPTH_BUFFER_BIT);
        for (const auto &node : gimbalArrows) {
            lix::renderNode(*objectShader, *node);
        }

        /*for(const auto& node : normalArrows)
        {
            lix::renderNode(*objectShader, *node);
        }*/
        lix::renderNode(*objectShader, *normalArrows.back());
    }

    if (showDialogue) {
        glClear(GL_DEPTH_BUFFER_BIT);
        glDisable(GL_CULL_FACE);
        hudShader->bind();
        hudShader->setUniform("u_time", lix::Time::seconds());
        for (const auto &node : hudObjects) {
            lix::renderNode(*hudShader, *node);
        }
    }
    textRendering->render();
}

void App::onSubjectTransformed(lix::Node *subject,
                               lix::Editor::Transformation transformation) {}

bool App::onKeyDown(lix::KeySym key, lix::KeyMod mod) {
    if (console->onKeyDown(key, mod)) {
        return false;
    }

    lix::Editor::onKeyDown(key, mod);

    auto &player = characterControllers.front();
    switch (key) {
    case SDLK_SPACE:
        player->jump();
        return true;
    case SDLK_w:
        player->moveForward();
        return true;
    case SDLK_a:
        player->moveLeft();
        return true;
    case SDLK_s:
        player->moveBackward();
        return true;
    case SDLK_d:
        player->moveRight();
        return true;
    case SDLK_x:
        xrayMode = !xrayMode;
        return true;
    }
    return false;
}

bool App::onKeyUp(lix::KeySym key, lix::KeyMod mod) {
    if (console->onKeyUp(key, mod)) {
        return false;
    }
    lix::Editor::onKeyUp(key, mod);
    auto &player = characterControllers.front();
    switch (key) {
    case SDLK_SPACE:
        player->stopJump();
        return true;
    case SDLK_w:
        player->stopForward();
        return true;
    case SDLK_a:
        player->stopLeft();
        return true;
    case SDLK_s:
        player->stopBackward();
        return true;
    case SDLK_d:
        player->stopRight();
        return true;
    }
    return false;
}

bool App::onMouseMove(float x, float y, float xrel, float yrel) {
    if (console->onMouseMove(x, y, xrel, yrel)) {
        return false;
    }
    lix::Editor::onMouseMove(x, y, xrel, yrel);
    characterControllers.front()->rotate(-xrel / WINDOW_X * 4.0f);
    cameraHeight += yrel / WINDOW_Y * 4.0f;
    cameraHeight = std::max(-6.0f, std::min(6.0f, cameraHeight));
    return false;
}