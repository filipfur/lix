#include <fstream>

#include "glapplication.h"
#include "glarrow.h"
#include "gleditor.h"
#include "glgeometry.h"
#include "glrendering.h"
#include "glshaderprogram.h"
#include "glvertexarray.h"
#include "json.h"
#include "primer.h"

#include "aabb.h"
#include "capsule.h"
#include "collision.h"
#include "convexhull.h"
#include "polygon.h"
#include "sphere.h"

#define RUNONCE(call)                                                          \
    do {                                                                       \
        static bool first = true;                                              \
        if (first) {                                                           \
            call;                                                              \
            first = false;                                                     \
        }                                                                      \
    } while (0);

float testRadii{0.09f};

static constexpr float WINDOW_X = 1080;
static constexpr float WINDOW_Y = 720;
static constexpr float NEAR = 0.01;
static constexpr float FAR = 100;
static const glm::mat4 PERSPECTIVE{
    glm::perspective(45.0f, WINDOW_X / WINDOW_Y, NEAR, FAR)};
static const glm::vec2 RESOLUTION{WINDOW_X, WINDOW_Y};

const char *vertexSource = LIX_SHADER_VERSION R"(
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

const char *fragmentSource = LIX_SHADER_VERSION R"(
    precision highp float;

    in vec2 texCoords;

    out vec4 fragColor;

    uniform sampler2D u_texture;
    uniform vec4 u_base_color;

    void main()
    {
        fragColor = u_base_color;
    }
)";

static std::vector<glm::vec3> computeMinkowski(lix::Polygon &polyA,
                                               lix::Polygon &polyB) {
    const auto &v0 = polyA.transformedPoints();
    const auto &v1 = polyB.transformedPoints();
    size_t n = v0.size();
    size_t m = v1.size();
    std::vector<glm::vec3> minkowski(n * m);

    for (size_t i{0}; i < n; ++i) {
        for (size_t j{0}; j < m; ++j) {
            minkowski[i * m + j] = v0[j] - v1[i];
        }
    }
    return minkowski;
}

class App : public lix::Application, public lix::Editor {
  public:
    App()
        : lix::Application{WINDOW_X, WINDOW_Y, "cdet"},
          lix::Editor{PERSPECTIVE, RESOLUTION} {}

    virtual void init() override;
    virtual void tick(float dt) override;
    virtual void draw() override;

    void postInit();

    void createPolygon(const std::vector<glm::vec3> &points,
                       const std::vector<lix::TRS> &frames, lix::Color dotColor,
                       lix::Color lineColor) {
        printf("creating polygon of: %zu\n", points.size());
        auto [vs, is] = lix::cubes_at_points(points);
        auto node = std::make_shared<lix::Node>();
        nodes.push_back(node);
        node->setMesh(std::make_shared<lix::Mesh>(
            std::make_shared<lix::VAO>(lix::Attributes{lix::Attribute::VEC3},
                                       vs, is),
            std::make_shared<lix::Material>(dotColor)));
        auto geomNode = std::make_shared<lix::Node>();
        geomNode->setMesh(std::make_shared<lix::Mesh>(
            std::make_shared<lix::VAO>(lix::Attributes{lix::Attribute::VEC3},
                                       points),
            std::make_shared<lix::Material>(lineColor *
                                            lix::Color::opacity(0.5f))));
        node->appendChild(geomNode);
        node->setTranslation(frames.front().translation());
        node->setRotation(frames.front().rotation());
        node->setScale(frames.front().scale());
        polygons.push_back(std::make_shared<lix::Polygon>(node.get(), points));
        allFrames.push_back(frames);
        curFrameIt.push_back(allFrames.back().begin());

        auto &poly = polygons.back();
        auto aabb = std::make_shared<lix::AABB>(poly->trs(), poly.get());
        poly->setSimplified(aabb);
    }

    void createSphere(char op, const std::vector<lix::TRS> &frames,
                      float radii) {
        allFrames.push_back(frames);
        curFrameIt.push_back(allFrames.back().begin());
        auto shape =
            std::make_shared<lix::Sphere>(&(*curFrameIt.back()), radii);
        if (op == '+') {
            shapes.back()->addPositive(shape);
        } else if (op == '-') {
            shapes.back()->addNegative(shape);
        } else {
            shapes.push_back(shape);
        }
    }

    void createAABB(char op, const std::vector<lix::TRS> &frames,
                    const glm::vec3 &min, const glm::vec3 &max) {
        allFrames.push_back(frames);
        curFrameIt.push_back(allFrames.back().begin());
        auto shape =
            std::make_shared<lix::AABB>(&(*curFrameIt.back()), min, max);
        if (op == '+') {
            shapes.back()->addPositive(shape);
        } else if (op == '-') {
            shapes.back()->addNegative(shape);
        } else {
            shapes.push_back(shape);
        }
    }

    void createCapsule(char op, const std::vector<lix::TRS> &frames,
                       const glm::vec3 &a, const glm::vec3 &b, float radii,
                       bool caps) {
        allFrames.push_back(frames);
        curFrameIt.push_back(allFrames.back().begin());
        auto shape = std::make_shared<lix::Capsule>(&(*curFrameIt.back()), a, b,
                                                    radii, caps);
        if (op == '+') {
            shapes.back()->addPositive(shape);
        } else if (op == '-') {
            shapes.back()->addNegative(shape);
        } else {
            shapes.push_back(shape);
        }
    }

    virtual void onSubjectTransformed(lix::Node *subject,
                                      Transformation transformation) override;

    void incrementGJK(lix::Polygon &polyA, lix::Polygon &polyB);
    void initGJK();
    void incrementEPA(lix::Polygon &polyA, lix::Polygon &polyB);

    std::vector<std::shared_ptr<lix::Node>> nodes;
    std::vector<std::vector<lix::TRS>> allFrames;
    std::vector<std::vector<lix::TRS>::iterator> curFrameIt;
    std::vector<std::shared_ptr<lix::Polygon>> polygons;
    lix::ShaderProgramPtr objectShader;
    std::vector<std::shared_ptr<lix::Node>> gimbalArrows;
    std::vector<std::shared_ptr<lix::Node>> gjkArrows;
    std::vector<std::shared_ptr<lix::Node>> convexHullArrows;
    std::vector<glm::vec3> gjkSimplex;
    glm::vec3 gjkDirection;
    lix::Collision gjkCollision;
    lix::gjk_state gjkState{lix::gjk_state::INCREMENTING};
    std::unique_ptr<lix::ConvexHull> epaCH;
    bool epaConverged{false};
    lix::Node shapeC;
    lix::Node shapeEPA;
    std::vector<glm::vec3> triangleTestVertices;
    lix::Node triangleTestNode;
    std::vector<std::shared_ptr<lix::Shape>> shapes;
    std::shared_ptr<lix::VAO> cloudVAO;
    std::shared_ptr<lix::VAO> convexHullVAO;
};

char operatorOperandus = '0';

void parseJSON(App &app, const char *file,
               std::list<std::vector<glm::vec3>> &allPoints) {
    std::ifstream ifs(file);
    json::Json json;
    if (ifs) {
        ifs >> json;
        auto &points = allPoints.emplace_back();
        std::vector<lix::TRS> frames;
        glm::vec3 translation{};
        glm::quat rotation{};
        glm::vec3 scale{};
        for (const auto &frameObj : json["frames"]) {
            const auto &t = frameObj["translation"];
            const auto &s = frameObj["scale"];
            translation = glm::vec3{t[0].get<float>(), t[1].get<float>(),
                                    t[2].get<float>()};
            if (frameObj.contains("euler")) {
                const auto &r = frameObj["euler"];
                rotation = glm::angleAxis(glm::radians(r[0].get<float>()),
                                          glm::vec3{1.0f, 0.0f, 0.0f}) *
                           glm::angleAxis(glm::radians(r[1].get<float>()),
                                          glm::vec3{0.0f, 1.0f, 0.0f}) *
                           glm::angleAxis(glm::radians(r[2].get<float>()),
                                          glm::vec3{0.0f, 0.0f, 1.0f});
            } else {
                const auto &r = frameObj["rotation"];
                rotation = glm::quat{r[0].get<float>(), r[1].get<float>(),
                                     r[2].get<float>(), r[3].get<float>()};
            }
            scale = glm::vec3{s[0].get<float>(), s[1].get<float>(),
                              s[2].get<float>()};
            frames.emplace_back(translation, rotation, scale);
        }

        std::string type{"polygon"};
        if (json.contains("type")) {
            type = json.at("type").value();
        }

        if (type == "polygon") {
            for (const auto &p : json["points"]) {
                points.emplace_back(p[0].get<float>(), p[1].get<float>(),
                                    p[2].get<float>());
            }
            app.createPolygon(points, frames, lix::Color::white,
                              lix::Color::yellow);
        } else if (type == "sphere") {
            float radii = json["radii"].get<float>();
            app.createSphere(operatorOperandus, frames, radii);
            printf("creating sphere: %.2f\n", radii);
        } else if (type == "aabb") {
            const auto &maxObj = json["max"];
            const auto &minObj = json["min"];

            glm::vec3 max{maxObj[0].get<float>(), maxObj[1].get<float>(),
                          maxObj[2].get<float>()};
            glm::vec3 min{minObj[0].get<float>(), minObj[1].get<float>(),
                          minObj[2].get<float>()};

            app.createAABB(operatorOperandus, frames, min, max);
        } else if (type == "capsule") {
            const auto &aObj = json["a"];
            const auto &bObj = json["b"];
            float radii = json["radii"].get<float>();

            bool caps{true};
            if (json.contains("caps") && (json["caps"].value() == "false" ||
                                          json["caps"].value() == "0")) {
                caps = false;
            }

            glm::vec3 a{aObj[0].get<float>(), aObj[1].get<float>(),
                        aObj[2].get<float>()};
            glm::vec3 b{bObj[0].get<float>(), bObj[1].get<float>(),
                        bObj[2].get<float>()};
            app.createCapsule(operatorOperandus, frames, a, b, radii, caps);
        } else {
            throw std::runtime_error("unkown type: " + type);
        }
    }
    ifs.close();
}

void parseBinary(App &app, const char *file,
                 std::list<std::vector<glm::vec3>> &allPoints) {
    std::ifstream ifs{file, std::ios::binary};
    if (!ifs) {
        return;
    }
    size_t n;
    ifs.read((char *)&n, sizeof(size_t));
    size_t numBytes = static_cast<int>(n * sizeof(glm::vec3));
    char arr[numBytes];
    ifs.read(arr, numBytes);
    auto &points =
        allPoints.emplace_back((glm::vec3 *)arr, (glm::vec3 *)arr + n);
    std::vector<lix::TRS> frames;
    ifs.read((char *)&n, sizeof(size_t));
    glm::vec3 pos;
    glm::quat rot;
    glm::vec3 scl;
    for (int i{0}; i < n; ++i) {
        ifs.read((char *)&pos, sizeof(glm::vec3));
        ifs.read((char *)&rot, sizeof(glm::quat));
        ifs.read((char *)&scl, sizeof(glm::vec3));
        frames.emplace_back(pos, rot, scl);
    }
    app.createPolygon(points, frames, lix::Color::white, lix::Color::yellow);
    unsigned char sanity;
    ifs >> sanity;
    assert(sanity == 0xAB);
    ifs.close();
}

int main(int argc, char *argv[]) {
    App app;
    app.initialize();
    static std::list<std::vector<glm::vec3>> allPoints;
    if (argc > 1) {
        for (size_t i{1}; i < argc; ++i) {
            const std::string fileName{argv[i]};
            if (fileName == "+" || fileName == "-") {
                operatorOperandus = fileName[0];
                continue;
            }
            const std::string fileExt = fileName.substr(fileName.rfind('.'));
            if (fileExt == ".json") {
                parseJSON(app, argv[i], allPoints);
            } else if (fileExt == ".bin") {
                parseBinary(app, argv[i], allPoints);
            } else {
                printf("ERROR: unkown file format: %s", fileExt.c_str());
            }
            operatorOperandus = '0';
        }
    } else {
        /*const std::vector<glm::vec3> points = {
            {1.0f, 1.0f, 1.0f},
            {-1.0f, 2.0f, 2.0f},
            {-1.0f, -1.0f, -1.0f}
        };
        app.createPolygon(points, {lix::TRS{glm::vec3{0.0f, 0.0f, 0.0f}}},
        lix::Color::white, lix::Color::yellow);*/
        // parseJSON(app, "tools/cdet/poly.json", allPoints);
        static const std::vector<glm::vec3> points = {
            {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, +1.0f}, {-1.0f, +1.0f, +1.0f},
            {+1.0f, -1.0f, -1.0f}, {+1.0f, -1.0f, +1.0f}, {+1.0f, +1.0f, -1.0f},
            {-1.0f, +1.0f, -1.0f}, {+1.0f, +1.0f, +1.0f}};
        app.createPolygon(points, {lix::TRS{glm::vec3{0.0f, 0.0f, 0.0f}}},
                          lix::Color::white, lix::Color::yellow);
    }
    if (app.polygons.size() == 2) {
        auto minkowski = computeMinkowski(*app.polygons[0], *app.polygons[1]);
        auto &unique = allPoints.emplace_back(lix::uniqueVertices(minkowski));
        /*std::vector<glm::vec4> v{unique.size()};
        std::transform(unique.begin(), unique.end(), v.begin(), [](const auto&
        p){ return glm::vec4(p, glm::dot(p, glm::vec3{1.0f, 0.0f, 0.0f})); });
        std::sort(v.begin(), v.end(), [](const auto& p0, const auto& p1) {
        return p1.w < p0.w; }); std::transform(v.begin(), v.end(),
        unique.begin(), [](const auto& p) { return glm::vec3(p.x, p.y,
        p.z);});*/
        printf("minkowski verts: %zu, unique verts: %zu\n", minkowski.size(),
               unique.size());
        app.createPolygon(unique, {lix::TRS{}}, lix::Color::cyan,
                          lix::Color::black);
    }
    app.run();
    return 0;
}

void App::incrementGJK(lix::Polygon &polyA, lix::Polygon &polyB) {
    if (gjkState != lix::gjk_state::INCREMENTING) {
        printf("gjk already converged, reset with [r]\n");
        return;
    }
    gjkState = lix::gjk_increment(polyA, polyB, gjkSimplex, gjkDirection,
                                  &gjkCollision);

    if (gjkState == lix::gjk_state::COLLISION) {
    }

    printf("#simplex=%zu gjk_state=", gjkSimplex.size());
    switch (gjkState) {
    case lix::gjk_state::INCREMENTING:
        printf("INCREMENTING\n");
        break;
    case lix::gjk_state::COLLISION:
        printf("COLLISION\n");
        break;
    case lix::gjk_state::NO_COLLISION:
        printf("NO_COLLISION\n");
        break;
    case lix::gjk_state::ERROR:
        printf("ERROR\n");
        break;
    }

    glm::vec3 up{0.0f, 1.0f, 0.0f};

    gjkArrows.clear();
    if (gjkSimplex.size() > 1) {
        auto spa00 = lix::getSupportPointOfA(polyA, gjkSimplex[0]);
        auto spa01 = lix::getSupportPointOfA(polyA, gjkSimplex[1]);

        auto spb00 = lix::getSupportPointOfB(polyB, gjkSimplex[0]);
        auto spb01 = lix::getSupportPointOfB(polyB, gjkSimplex[1]);

        gjkArrows.insert(
            gjkArrows.end(),
            {
                lix::arrow(lix::Color::cyan, gjkSimplex[0],
                           glm::normalize(gjkSimplex[1] - gjkSimplex[0])),
                lix::arrow(lix::Color::cyan, gjkSimplex[1], up),

                lix::arrow(lix::Color::magenta, spa00,
                           glm::normalize(spa01 - spa00)),
                lix::arrow(lix::Color::magenta, spa01, up),

                lix::arrow(lix::Color::magenta, spb00,
                           glm::normalize(spb01 - spb00)),
                lix::arrow(lix::Color::magenta, spb01, up),
            });
    }
    if (gjkSimplex.size() > 2) {
        auto spa01 = lix::getSupportPointOfA(polyA, gjkSimplex[1]);
        auto spa02 = lix::getSupportPointOfA(polyA, gjkSimplex[2]);

        auto spb01 = lix::getSupportPointOfB(polyB, gjkSimplex[1]);
        auto spb02 = lix::getSupportPointOfB(polyB, gjkSimplex[2]);

        gjkArrows.at(gjkArrows.size() - 5)
            ->setRotation(lix::directionToQuat(
                glm::normalize(gjkSimplex[2] - gjkSimplex[1])));
        gjkArrows.at(gjkArrows.size() - 3)
            ->setRotation(lix::directionToQuat(glm::normalize(spa02 - spa01)));
        gjkArrows.at(gjkArrows.size() - 1)
            ->setRotation(lix::directionToQuat(glm::normalize(spb02 - spb01)));

        const glm::vec3 &A = gjkSimplex[2];
        const glm::vec3 &B = gjkSimplex[1];
        const glm::vec3 &C = gjkSimplex[0];
        const glm::vec3 AB = B - A;
        const glm::vec3 AC = C - A;
        glm::vec3 T;
        if (glm::dot(AB, -A) < glm::dot(AC, -A)) {
            float t = -(glm::dot(AB, A) / glm::dot(AB, AB));
            T = A + AB * t;
        } else {
            float t = -(glm::dot(AC, A) / glm::dot(AC, AC));
            T = A + AC * t;
        }

        gjkArrows.insert(gjkArrows.end(),
                         {lix::arrow(lix::Color::cyan, gjkSimplex[2], up),

                          lix::arrow(lix::Color::magenta, spa02, up),

                          lix::arrow(lix::Color::magenta, spb02, up),

                          lix::arrow(lix::Color::red, T, glm::normalize(-T))});
        shapeC.setMesh(std::make_shared<lix::Mesh>(
            std::make_shared<lix::VAO>(lix::Attributes{lix::Attribute::VEC3},
                                       gjkSimplex,
                                       std::vector<GLuint>{0, 1, 2}),
            std::make_shared<lix::Material>(lix::Color::cyan *
                                            lix::Color::opacity(0.33f))));
    }
    if (gjkSimplex.size() > 3) {
        auto spa02 = lix::getSupportPointOfA(polyA, gjkSimplex[2]);
        auto spa03 = lix::getSupportPointOfA(polyA, gjkSimplex[3]);

        auto spb02 = lix::getSupportPointOfB(polyB, gjkSimplex[2]);
        auto spb03 = lix::getSupportPointOfB(polyB, gjkSimplex[3]);

        gjkArrows.at(gjkArrows.size() - 4)
            ->setRotation(lix::directionToQuat(
                glm::normalize(gjkSimplex[3] - gjkSimplex[2])));
        gjkArrows.at(gjkArrows.size() - 3)
            ->setRotation(lix::directionToQuat(glm::normalize(spa03 - spa02)));
        gjkArrows.at(gjkArrows.size() - 2)
            ->setRotation(lix::directionToQuat(glm::normalize(spb03 - spb02)));

        gjkArrows.insert(gjkArrows.end(),
                         {
                             lix::arrow(lix::Color::cyan, gjkSimplex[3], up),

                             lix::arrow(lix::Color::magenta, spa03, up),

                             lix::arrow(lix::Color::magenta, spb03, up),
                         });
        shapeC.setMesh(std::make_shared<lix::Mesh>(
            std::make_shared<lix::VAO>(
                lix::Attributes{lix::Attribute::VEC3}, gjkSimplex,
                std::vector<GLuint>{0, 1, 2, 1, 3, 2, 0, 2, 3, 0, 3, 1}),
            std::make_shared<lix::Material>(lix::Color::cyan *
                                            lix::Color::opacity(0.33f))));
    }
}

void App::incrementEPA(lix::Polygon &polyA, lix::Polygon &polyB) {
    if (gjkState != lix::gjk_state::COLLISION) {
        printf("cant start epa because gjk has not detected collision\n");
        return;
    }
    if (epaConverged) {
        printf("epa already converged\n");
        return;
    }
    if (epaCH == nullptr) {
        epaCH.reset(new lix::ConvexHull(gjkSimplex));
        gjkArrows.clear();
    }
    epaConverged = lix::epa_increment(polyA, polyB, *epaCH, &gjkCollision);

    auto [vs, is] = epaCH->meshData();
    shapeEPA.setMesh(std::make_shared<lix::Mesh>(
        std::make_shared<lix::VAO>(lix::Attributes{lix::Attribute::VEC3}, vs,
                                   is),
        std::make_shared<lix::Material>(lix::Color::green *
                                        lix::Color::opacity(0.5f))));

    if (epaConverged) {
        printf("penetration: %.3f\n", gjkCollision.penetrationDepth);
        gjkArrows.push_back(lix::arrow(
            lix::Color::red, gjkCollision.contactPoint, gjkCollision.normal));
    }
}

bool _popAlongDirection(std::vector<glm::vec3> &s, const glm::vec3 &D,
                        glm::vec3 &rval) {
    int index{-1};
    float maxValue{-FLT_MAX}; // glm::dot(s[0], D)};

    assert(s.size() != 0);

    for (size_t i{0}; i < s.size(); ++i) {
        float value = glm::dot(s[i], D);
        if (value > maxValue) {
            index = static_cast<int>(i);
            maxValue = value;
        }
    }

    if (index < 0) {
        return false;
    }
    rval = s[index];
    s.erase(s.begin() + index);
    return true;
}

void App::init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    setInputAdapter(this);

    objectShader.reset(new lix::ShaderProgram(vertexSource, fragmentSource));

    camera().setupUBO({objectShader.get()});

    // create gimbal
    gimbalArrows.insert(
        gimbalArrows.end(),
        {lix::arrow(lix::Color::red, glm::vec3{0.0f, 0.0f, 0.0f},
                    glm::vec3{1.0f, 0.0f, 0.0f}),
         lix::arrow(lix::Color::green, glm::vec3{0.0f, 0.0f, 0.0f},
                    glm::vec3{0.0f, 1.0f, 0.0f}),
         lix::arrow(lix::Color::blue, glm::vec3{0.0f, 0.0f, 0.0f},
                    glm::vec3{0.0f, 0.0f, 1.0f})});

    setOnKeyDown(SDLK_n, [this](auto, auto mods) {
        if (mods & KMOD_SHIFT) {
            int maxCounter = 1000;
            do {
                incrementGJK(*polygons[0], *polygons[1]);
                if (--maxCounter < 0) {
                    printf("ERROR: max counter hit during gjk<1>\n");
                    break;
                }
            } while (gjkState == lix::gjk_state::INCREMENTING);
        }
        incrementGJK(*polygons[0], *polygons[1]);
    });
    setOnKeyDown(SDLK_m, [this](auto, auto mods) {
        if (mods & KMOD_SHIFT) {
            int maxCounter = 1000;
            while (gjkState == lix::gjk_state::INCREMENTING) {
                incrementGJK(*polygons[0], *polygons[1]);
                if (--maxCounter < 0) {
                    printf("ERROR: max counter hit during gjk<2>\n");
                    break;
                }
            }
            maxCounter = 1000;
            do {
                incrementEPA(*polygons[0], *polygons[1]);
                if (--maxCounter < 0) {
                    printf("ERROR: max counter hit during epa\n");
                    break;
                }
            } while (!epaConverged);
        }
        incrementEPA(*polygons[0], *polygons[1]);
    });
    setOnKeyDown(SDLK_r, [this](auto, auto) { initGJK(); });
    setOnKeyDown(SDLK_f, [this](auto, auto) {
        for (size_t i{0}; i < allFrames.size(); ++i) {
            ++curFrameIt[i];
            if (curFrameIt[i] == allFrames[i].end()) {
                curFrameIt[i] = allFrames[i].begin();
            }
            polygons[i]->trs()->setTranslation(curFrameIt[i]->translation());
            polygons[i]->trs()->setRotation(curFrameIt[i]->rotation());
            polygons[i]->trs()->setScale(curFrameIt[i]->scale());
        }
        initGJK();
    });
    setOnKeyDown(SDLK_g, [this](auto, auto) {
        if (lix::collides(*polygons[0], *polygons[1], &gjkCollision)) {
            printf("penetration: %f\n", gjkCollision.penetrationDepth);
        }
    });

    setOnKeyDown(SDLK_c, [this](auto, auto) {
        static std::shared_ptr<lix::ConvexHull> convexHull = nullptr;
        static std::list<lix::Face *> Q;
        if (polygons.size() > 0) {
            auto &poly = polygons.front();
            static std::vector<glm::vec3> P{poly->points().begin(),
                                            poly->points().end()};
            if (convexHull == nullptr) {
                printf("created convex hull!\n");
                convexHull.reset(new lix::ConvexHull(P));
                std::for_each(convexHull->begin(), convexHull->end(),
                              [&](lix::Face &face) { Q.push_back(&face); });
            }
            convexHullVAO.reset(new lix::VAO(
                lix::Attributes{lix::Attribute::VEC3}, convexHull->points()));
        }
    });

    setOnKeyDown(SDLK_p, [this](auto, auto mods) {
        std::vector<glm::vec3> pts;
        lix::TRS trs;
        static lix::AABB aabb{&trs, glm::vec3{-testRadii},
                              glm::vec3{testRadii}};
        static lix::Sphere sphere{&trs, testRadii};
        for (float z{-2.0f}; z <= 2.0f; z += 0.1f) {
            for (float y{-2.0f}; y <= 2.0f; y += 0.1f) {
                for (float x{-2.0f}; x <= 2.0f; x += 0.1f) {
                    glm::vec3 v{x, y, z};
                    trs.setTranslation(v);

                    lix::Shape *ptr = nullptr;
                    if (mods & KMOD_SHIFT) {
                        ptr = &sphere;
                    } else {
                        ptr = &aabb;
                    }

                    for (auto &shape : shapes) {
                        if (shape->test(*ptr)) {
                            pts.push_back(v);
                        }
                    }
                }
            }
        }
        auto [vs, is] = lix::cubes_at_points(pts, testRadii);
        cloudVAO.reset(
            new lix::VAO(lix::Attributes{lix::Attribute::VEC3}, vs, is));
    });

    triangleTestVertices.insert(
        triangleTestVertices.end(),
        {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}});

    setSubjectNode(&triangleTestNode);

    triangleTestNode.setMesh(std::make_shared<lix::Mesh>(
        std::make_shared<lix::VAO>(lix::Attributes{lix::Attribute::VEC3},
                                   triangleTestVertices),
        std::make_shared<lix::Material>(lix::Color::yellow)));
}

void App::initGJK() {
    lix::gjk_init();
    assert(polygons.size() > 1);
    gjkDirection = glm::normalize(polygons[1]->trs()->translation() -
                                  polygons[0]->trs()->translation());
    const glm::vec3 &D = gjkDirection;
    // printf("D=[%f %f %f]\n", D.x, D.y, D.z);
    gjkArrows.clear();
    shapeC.setMesh(nullptr);
    shapeEPA.setMesh(nullptr);
    gjkSimplex.clear();
    gjkState = lix::gjk_state::INCREMENTING;
    epaCH.reset();
    epaConverged = false;
}

void App::postInit() {
    printf("post init\n");
    // initGJK();
    // incrementGJK(*polygons[0], *polygons[1]);
}

void App::tick(float dt) {
    RUNONCE(postInit(););
    Editor::refresh(dt);
}

void App::draw() {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    objectShader->bind();
    for (const auto &node : nodes) {
        lix::renderNode(*objectShader, *node, false, true);
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        lix::renderNode(*objectShader, *node->childAt(0), false, true);
        glEnable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    if (shapeEPA.mesh()) {
        lix::renderNode(*objectShader, shapeEPA);
        const auto color = shapeEPA.mesh()->material()->baseColor();
        shapeEPA.mesh()->material()->setBaseColor(lix::Color::white);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        lix::renderNode(*objectShader, shapeEPA);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        shapeEPA.mesh()->material()->setBaseColor(color);
    } else {
        lix::renderNode(*objectShader, shapeC);
    }
    if (cloudVAO) {
        objectShader->setUniform("u_base_color", lix::Color::green.vec4());
        objectShader->setUniform("u_model", glm::mat4{1.0f});
        cloudVAO->bind();
        cloudVAO->draw();
    }

    if (convexHullVAO) {
        glEnable(GL_CULL_FACE);
        objectShader->setUniform("u_base_color", lix::Color::yellow *
                                                     lix::Color::opacity(0.5f));
        convexHullVAO->bind();
        convexHullVAO->draw();
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        objectShader->setUniform("u_base_color", lix::Color::magenta);
        convexHullVAO->bind();
        convexHullVAO->draw();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    for (const auto &node : gimbalArrows) {
        lix::renderNode(*objectShader, *node);
    }
    for (const auto &node : gjkArrows) {
        lix::renderNode(*objectShader, *node);
    }
    for (const auto &node : convexHullArrows) {
        lix::renderNode(*objectShader, *node);
    }

    /*glDisable(GL_CULL_FACE);
    const glm::vec3 a = triangleTestVertices[0] +
    triangleTestNode.translation(); const glm::vec3 b = triangleTestVertices[1]
    + triangleTestNode.translation(); const glm::vec3 c =
    triangleTestVertices[2] + triangleTestNode.translation(); bool inside =
    lix::pointInTriangle(glm::vec3{0.0f, 0.0f, 0.0f}, a, b, c);
    triangleTestNode.mesh()->material()->setBaseColor(inside ?
    lix::Color::yellow : lix::Color::red); lix::renderNode(*objectShader,
    triangleTestNode); glEnable(GL_CULL_FACE);*/
}

void App::onSubjectTransformed(lix::Node *subject,
                               lix::Editor::Transformation transformation) {}