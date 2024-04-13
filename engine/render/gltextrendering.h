#pragma once

#include <vector>
#include "gltext.h"
#include "glshaderprogram.h"

namespace lix
{
    class TextRendering
    {        
    public:
        TextRendering(const glm::vec2& resolution,
            const std::vector<std::shared_ptr<lix::Text>>& texts,
            std::shared_ptr<lix::ShaderProgram> shaderProgram=nullptr);

        void render();

        std::shared_ptr<lix::Text> text(size_t index);
        size_t count() const;

        std::shared_ptr<lix::ShaderProgram> shaderProgram()
        {
            return _shaderProgram;
        }

        const glm::vec2& resolution() const
        {
            return _resolution;
        }

    protected:
        void _renderText(lix::Text& text);

    private:
        glm::vec2 _resolution;
        std::shared_ptr<lix::ShaderProgram> _shaderProgram;
        const std::vector<std::shared_ptr<lix::Text>>& _texts;
    };
}