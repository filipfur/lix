#pragma once

#include <memory>
#include <string>

#include "glm/glm.hpp"

#include "glnode.h"
#include "glfont.h"
#include "glmesh.h"
#include "glcolor.h"

namespace lix
{
    class Text : public Node
    {
    public:
        enum Alignment {
            LEFT,
            CENTER
        };

        struct Properties
        {
            float textScale;
            Alignment alignment;
            float lineSpacing;
            float letterSpacing;
            Color textColor;
            Color borderColor;
            float borderWidth;
            float borderSmoothness;
        };

        struct PropBuilder : public Properties
        {
            PropBuilder();
            PropBuilder& setTextScale(float textScale);
            PropBuilder& setAlignment(Alignment alignment);
            PropBuilder& setLineSpacing(float lineSpacing);
            PropBuilder& setLetterSpacing(float letterSpacing);
            PropBuilder& setTextColor(const Color& textColor);
            PropBuilder& setBorderColor(const Color& borderColor);
            PropBuilder& setBorderWidth(float borderWidth);
            PropBuilder& setBorderSmoothness(float borderSmoothness);
        };
        
        static const Properties defaultProperties;

        Text(std::shared_ptr<lix::Font> font,
            const Properties& properties,
            const std::string& text);

        virtual ~Text() noexcept;
        
        Text(const Text& other) = delete;
        Text& operator=(const Text& other) = delete;

        std::shared_ptr<lix::Font> font();

        lix::VertexArray& vao();

        Properties& properties();

        lix::Text* setText(const std::string& text);
        const std::string& text() const;

    protected:
        void measureText();
        void initBuffers();

    private:
        std::shared_ptr<lix::Font> _font{nullptr};
        Properties _properties;
        std::string _text{""};
        lix::VertexArray _vao;
        float _width{0.0f};
        float _height{0.0f};
        float _yOffset{0.0f};
        std::vector<float> _vertices;
        std::vector<unsigned int> _indices;
        std::vector<glm::vec2> _letterXPositions;
        bool _bufferAllocated{false};
        std::vector<std::string> _lines;
        std::vector<float> _lineWidths;
    };

    using TextPtr = std::shared_ptr<lix::Text>;
}