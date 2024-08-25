#pragma once

#include "glshaderprogram.h"
#include "gltext.h"
#include <vector>

namespace lix {
class TextRendering {
  public:
    TextRendering(const glm::vec2 &resolution,
                  const std::vector<std::shared_ptr<lix::Text>> &texts,
                  std::shared_ptr<lix::ShaderProgram> shaderProgram = nullptr);

    void render();

    std::shared_ptr<lix::Text> text(size_t index);
    size_t count() const;

    std::shared_ptr<lix::ShaderProgram> shaderProgram() {
        return _shaderProgram;
    }

    const glm::vec2 &resolution() const { return _resolution; }

    const glm::mat4 &projection() const { return _projection; }

    const glm::mat4 &view() const { return _view; }

  protected:
    void _renderText(lix::Text &text);

  private:
    glm::vec2 _resolution;
    glm::mat4 _projection;
    glm::mat4 _view;
    std::shared_ptr<lix::ShaderProgram> _shaderProgram;
    const std::vector<std::shared_ptr<lix::Text>> &_texts;
};
} // namespace lix