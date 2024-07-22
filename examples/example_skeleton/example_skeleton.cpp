#include "glapplication.h"
#include "gleditor.h"

static constexpr float WINDOW_X = 1080;
static constexpr float WINDOW_Y = 720;
static constexpr float NEAR = 0.01;
static constexpr float FAR = 100;
static const glm::mat4 PERSPECTIVE{glm::perspective(45.0f, WINDOW_X / WINDOW_Y, NEAR, FAR)};
static const glm::vec2 RESOLUTION{WINDOW_X, WINDOW_Y};

class App : public lix::Application, public lix::Editor
{
public:
    App() : lix::Application{WINDOW_X, WINDOW_Y, "example_skeleton"}, lix::Editor{ PERSPECTIVE, RESOLUTION }
    {}

    virtual void init() override;
    virtual void tick(float dt) override;
    virtual void draw() override;

    virtual void onSubjectTransformed(lix::Node* subject, Transformation transformation) override;
};

int main(int argc, char* argv[])
{
    App app;
    app.run();
    return 0;
}

void App::init()
{
    setInputAdapter(this);
}

void App::tick(float dt)
{

}

void App::draw()
{

}

void App::onSubjectTransformed(lix::Node* subject, lix::Editor::Transformation transformation)
{

}