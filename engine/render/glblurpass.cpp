#include "glblurpass.h"

#include "glrendering.h"

namespace {
const char *blurVertSrc = LIX_SHADER_VERSION R"(
layout(location=0) in vec4 aVertex;

out vec2 texCoords;

void main()
{
  texCoords = aVertex.zw;
  gl_Position = vec4(aVertex.xy, 0.0, 1.0);
}
    )";

const char *blurFragSrc = LIX_SHADER_VERSION R"(
precision highp float;

in vec2 texCoords;
out vec4 FragColor;

uniform sampler2D image;
  
uniform bool horizontal;
const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{             
    vec2 tex_offset = 1.0 / vec2(textureSize(image, 0)); // gets size of single texel
    vec3 result = texture(image, texCoords).rgb * weight[0]; // current fragment's contribution
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, texCoords + vec2(tex_offset.x * float(i), 0.0)).rgb * weight[i];
            result += texture(image, texCoords - vec2(tex_offset.x * float(i), 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, texCoords + vec2(0.0, tex_offset.y * float(i))).rgb * weight[i];
            result += texture(image, texCoords - vec2(0.0, tex_offset.y * float(i))).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}
    )";

std::shared_ptr<lix::ShaderProgram> sharedShader() {
    static auto shader =
        std::make_shared<lix::ShaderProgram>(blurVertSrc, blurFragSrc);
    return shader;
}

} // namespace

lix::BlurPass::BlurPass(const glm::ivec2 &resolution)
    : BlurPass{resolution, sharedShader()} {}

lix::BlurPass::BlurPass(const glm::ivec2 &resolution,
                        std::shared_ptr<lix::ShaderProgram> shaderProgram)
    : _blurFBO{{resolution}, {resolution}},
      _shaderProgram{shaderProgram ? shaderProgram : sharedShader()} {
    for (int i{0}; i < 2; ++i) {
        _blurFBO[i].bind();
        _blurFBO[i].createTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA,
                                  GL_FLOAT);
        _blurFBO[i].unbind();
    }
}

void lix::BlurPass::blur(std::shared_ptr<lix::Texture> inputTexture) {
    _shaderProgram->bind();
    size_t amount = 10;
    bool horizontal = true, first_iteration = true;
    for (size_t i = 0; i < amount; i++) {
        _blurFBO[horizontal].bind();
        _shaderProgram->setUniform("horizontal", horizontal);
        if (first_iteration) {
            inputTexture->bind(GL_TEXTURE0);
            first_iteration = false;
        } else {
            _blurFBO[!horizontal].bindTexture(GL_COLOR_ATTACHMENT0,
                                              GL_TEXTURE0);
        }
        lix::renderScreen();
        horizontal = !horizontal;
    }
    _blurFBO[0].unbind();
}