#include "glapplication.h"
#include "glvertexarray.h"
#include "glshaderprogram.h"
#include "glcolor.h"
#include "gltexture.h"
#include "glrendering.h"
#include "gluniformbuffer.h"

#include "glm/glm.hpp"

#include "gltfloader.h"

#include "gen/objects/cube.h"
#include "gen/shaders/object_vert.h"

#include <iostream>

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 400

const float aspectRatio = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);

class App : public lix::Application
{
public:
    App(int windowX, int windowY, const char* title);

    virtual void init() override;

    virtual void tick(float dt) override;

    virtual void draw() override;

    static constexpr glm::vec3 upVector{0.0f, 1.0f, 0.0f};
    static constexpr glm::vec3 cameraTarget{0.0f, 0.0f, 0.0f};
    static constexpr glm::vec3 cameraPosition{10.0f, 4.0f, 10.0f};

    struct CameraBlock
    {
        glm::mat4 projection{glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 200.0f)};
        glm::mat4 view{glm::lookAt(cameraPosition, cameraTarget, upVector)};
        glm::vec3 eyePosition{cameraPosition};
        float alignment;
    } cameraBlock;
};

int main(int argc, char* argv[])
{
    static App app{SCREEN_WIDTH, SCREEN_HEIGHT, "Basic"};
    app.run();
    return 0;
}

App::App(int windowX, int windowY, const char* title) : Application{windowX, windowY, title} {}

void App::init()
{

}

void App::tick(float dt)
{

}

void App::draw()
{
    static lix::VAO vao{
        lix::Attributes{lix::VEC3, lix::VEC3},
        {
            0.0f, 1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        }
    };

    static lix::ShaderProgram shaderProgram{
        LIX_SHADER_VERSION R"(
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aColor;

out vec3 color;

void main()
{
  color = aColor;
  gl_Position = vec4(aPos, 1.0);
}
)", LIX_SHADER_VERSION R"(
precision highp float;

in vec3 color;

out vec4 fragColor;

void main()
{
    fragColor = vec4(color, 1.0);
}
)"
    };

        static lix::ShaderProgram textureShader{
        LIX_SHADER_VERSION R"(
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aTexCoords;

out vec2 texCoords;

void main()
{
  texCoords = aTexCoords;
  gl_Position = vec4(aPos * 0.5, 1.0);
}
)", LIX_SHADER_VERSION R"(
precision highp float;

uniform sampler2D u_texture;

in vec2 texCoords;

out vec4 fragColor;

void main()
{
    fragColor = texture(u_texture, texCoords);
}
)"
    };

    static std::shared_ptr<lix::Texture> texture = lix::Texture::Basic(lix::Color::red);

    shaderProgram.bind();
    vao.bind();
    vao.draw();

    texture->bind();
    //texture->setLodBias(0.5f);
    textureShader.bind();
    vao.bind();
    vao.draw();

    static lix::ShaderProgram objectShader{
        assets::shaders::object_vert,
        LIX_SHADER_VERSION R"(
precision highp float;

out vec4 fragColor;

uniform vec4 u_base_color;

void main()
{
    fragColor = u_base_color;
}
        )"};

    static lix::UniformBuffer cameraUBO{sizeof(CameraBlock), &cameraBlock, "CameraBlock", 0};
    static bool doOnce{true};
    if(doOnce)
    {
        std::cout << "sizeof(CameraBlock)=" << sizeof(CameraBlock) << std::endl;
        cameraUBO.bindShaders({&objectShader});
        doOnce = false;
    }

    static auto cube = gltf::loadNode(assets::objects::cube::Cube_node);
    cube->mesh()->material(0)->setBaseColor(lix::Color{lix::Color::magenta});
    objectShader.bind();
    lix::renderNode(objectShader, *cube);
}