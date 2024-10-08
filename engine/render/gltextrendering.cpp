#include "gltextrendering.h"

namespace {
const char *vertexSrcSdfText = LIX_SHADER_VERSION R"(
    layout (location = 0) in vec4 aVertex;

    uniform mat4 u_projection;
    uniform mat4 u_view;
    uniform mat4 u_model;

    out vec2 texCoords;
    out vec2 position;

    void main()
    {
        position = vec2(aVertex.x, -aVertex.y);
        texCoords = aVertex.zw;
        vec3 p0 = vec3(u_model * vec4(position, 0.0, 1.0));
        gl_Position = u_projection * u_view * vec4(p0, 1.0);
    }
    )";

const char *fragmentSrcSdfText = LIX_SHADER_VERSION R"(
    precision highp float;

    out vec4 FragColor;

    uniform vec4 u_color;
    uniform sampler2D u_texture;

    in vec2 texCoords;
    in vec2 position;

    void main()
    {    
        float sdfValue = texture(u_texture, texCoords).r;
        float border = 0.05;
        float alpha = smoothstep(0.5 - border, 0.5 + border, sdfValue);
        FragColor = vec4(u_color.rgb, alpha);
    }
    )";
} // namespace

lix::TextRendering::TextRendering(
    const glm::vec2 &resolution,
    const std::vector<std::shared_ptr<lix::Text>> &texts,
    std::shared_ptr<lix::ShaderProgram> shaderProgram)
    : _resolution{resolution}, _shaderProgram{shaderProgram}, _texts{texts} {
    if (_shaderProgram == nullptr) {
        _shaderProgram.reset(
            new lix::ShaderProgram(vertexSrcSdfText, fragmentSrcSdfText));
    }
    _projection =
        glm::ortho(-resolution.x * 0.5f, resolution.x * 0.5f,
                   -resolution.y * 0.5f, resolution.y * 0.5f, -10.0f, 10.0f);
    _view =
        glm::lookAt(glm::vec3{0.0f, 0.0f, 1.0f}, glm::vec3{0.0f, 0.0f, 0.0f},
                    glm::vec3{0.0f, 1.0f, 0.0f});
    _shaderProgram->bind();
    _shaderProgram->setUniform("u_projection", _projection);
    _shaderProgram->setUniform("u_view", _view);
}

void lix::TextRendering::render() {
    glDisable(GL_DEPTH_TEST);
    _shaderProgram->bind();
    for (auto &text : _texts) {
        if (text->visible()) {
            _renderText(*text);
        }
    }
    glEnable(GL_DEPTH_TEST);
}

std::shared_ptr<lix::Text> lix::TextRendering::text(size_t index) {
    auto it = _texts.begin();
    std::advance(it, index);
    return *it;
}

size_t lix::TextRendering::count() const { return _texts.size(); }

void lix::TextRendering::_renderText(lix::Text &text) {
    auto &props = text.properties();
    _shaderProgram->setUniform("u_color", props.textColor.vec4());
    if (text.properties().borderColor.vec4().w > 0) {
        _shaderProgram->setUniform("u_border_color", props.borderColor.vec4());
        _shaderProgram->setUniform("u_border_width", props.borderWidth);
        _shaderProgram->setUniform("u_edge_smoothness", props.borderSmoothness);
    }
    _shaderProgram->setUniform("u_model", text.globalMatrix());
    text.font()->texture().bind(GL_TEXTURE0);
    text.vao().bind();
    text.vao().draw();
}