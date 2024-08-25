#pragma once

#include "glframebuffer.h"
#include "glshaderprogram.h"

namespace lix {
class BlurPass {
  public:
    BlurPass(const glm::ivec2 &resolution);

    BlurPass(const glm::ivec2 &resolution,
             std::shared_ptr<lix::ShaderProgram> shaderProgram);

    void blur(std::shared_ptr<lix::Texture> inputTexture);

    std::shared_ptr<lix::Texture> outputTexture() {
        return _blurFBO[0].texture(GL_COLOR_ATTACHMENT0);
    }

  private:
    lix::FrameBuffer _blurFBO[2];
    std::shared_ptr<lix::ShaderProgram> _shaderProgram;
};
} // namespace lix