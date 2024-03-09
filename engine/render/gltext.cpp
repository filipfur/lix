#include "gltext.h"

lix::Text::Text(std::shared_ptr<lix::Font> font,
    const lix::Text::Properties& properties,
    const std::string& text)
    : _font{font}, _properties{properties}, _text{text}, _vao{GL_TRIANGLES}
{
    _vao.bind();
    _vao.createVbo(GL_DYNAMIC_DRAW, {lix::Attribute::VEC4}, std::vector<GLfloat>{
        0.0f, 0.0f, 0.0f, 0.0f,
        300.0f, 0.0f, 1.0f, 0.0f,
        300.0f, 300.0f, 1.0f, 1.0f
    });
    _vao.createEbo(GL_DYNAMIC_DRAW, std::vector<GLuint>{0, 1, 2});
    initBuffers();
}

lix::Text::~Text() noexcept
{

}

std::shared_ptr<lix::Font> lix::Text::font()
{
    return _font;
}

lix::VertexArray& lix::Text::vao()
{
    return _vao;
}

lix::Text::Properties& lix::Text::properties()
{
    return _properties;
}

lix::Text* lix::Text::setText(const std::string& text)
{
    _text = text;
    _vao.bind();
    initBuffers();
    return this;
}

void lix::Text::measureText()
{
    _width = 0.0f;
    _height = 0.0f;
    _lines.clear();
    _lineWidths.clear();

    float x{0.0f};
    std::string line{""};
    float line0OriginY{0.0f};
    float y{0.0f};
    float lineNY2{0.0f};
    for(size_t i{0}; i < _text.length(); ++i)
    {
        if(_text[i] == '\n')
        {
            _width = std::max(_width, x);
            _lineWidths.push_back(x);
            _lines.push_back(line);
            line = "";
            x = 0;
            y += _font->maxCharacterHeight() * _properties.lineSpacing * _properties.textScale;
            continue;
        }
        auto c = _font->character(_text[i]);
        if(_lines.size() == 0)
        {
            line0OriginY = std::max(c.originY, line0OriginY);
        }
        line += _text[i];
        x += c.advance * _properties.textScale * _properties.letterSpacing;
        float y2 = y - c.originY * _properties.textScale + c.height * _properties.textScale;
        lineNY2 = std::max(lineNY2, y2);
    }

    if(x > 0)
    {
        _width = std::max(_width, x);
        _lineWidths.push_back(x);
        _lines.push_back(line);
    }

    _yOffset = -line0OriginY * _properties.textScale;
    _height = lineNY2 - _yOffset;
}


void lix::Text::initBuffers()
{
    float x = 0.0f;
    float y = 0.0f;//fontsize / 2.0f;

    _vertices.clear();
    _indices.clear();
    _letterXPositions.clear();

    measureText();

    GLuint curidx = 0;//i * 4;

    switch(_properties.alignment)
    {
        case Alignment::LEFT:
            break;
        case Alignment::CENTER:
            y = -_yOffset - _height * 0.5f;
            break;
        default:
            break;
    }

    for(size_t lineNo = 0; lineNo < _lines.size(); ++lineNo)
    {
        const auto& line = _lines[lineNo];

        switch(_properties.alignment)
        {
            case Alignment::LEFT:
                x = 0.0f;
                break;
            case Alignment::CENTER:
                x = (_width - _lineWidths[lineNo]) / 2.0f - _width * 0.5f;
                break;
            default:
                break;
        }

        for(size_t i = 0; i < line.length(); ++i)
        {
            auto c = _font->character(line[i]);
            float x0 = x - c.originX * _properties.textScale;
            float y0 = y - c.originY * _properties.textScale;
            float s0 = c.x / _font->width();
            float t0 = c.y / _font->height();

            float w = c.width * _properties.textScale;

            float x1 = x - c.originX * _properties.textScale + w;
            float s1 = (c.x + c.width) / _font->width();

            float y2 = y - c.originY * _properties.textScale + c.height * _properties.textScale;
            float t2 = (c.y + c.height) / _font->height();


            _vertices.insert(_vertices.end(), { 
                x0, y0, s0, t0,
                x1, y0, s1, t0,  
                x0, y2, s0, t2,  
                x1, y2, s1, t2});

            // p0 --- p1
            // | \     |
            // |   \   |
            // |     \ |
            // p2 --- p3

            _indices.insert(_indices.end(), {
                curidx+0, curidx+2, curidx+1,  // first triangle
                curidx+1, curidx+2, curidx+3}); // second triangle
            curidx += 4;
            float oldX = x;
            x += c.advance * _properties.textScale * _properties.letterSpacing;
            _letterXPositions.push_back(glm::vec2{oldX, x});
        }

        y += _font->maxCharacterHeight() * _properties.lineSpacing * _properties.textScale;
    }
    _bufferAllocated = false;

    _vao.vbo(0)->bind();
    _vao.vbo(0)->bufferData(_vertices);
    _vao.ebo()->bind();
    _vao.ebo()->bufferData(_indices);

    invalidate();
}

const lix::Text::Properties lix::Text::defaultProperties{1.0f, Alignment::LEFT, 1.0f, 1.0f, lix::Color::white, lix::Color::black};

lix::Text::PropBuilder::PropBuilder() : lix::Text::Properties{1.0f, Alignment::LEFT, 1.0f, 1.0f, lix::Color::white, lix::Color::black}
{
    
}

lix::Text::PropBuilder& lix::Text::PropBuilder::setTextScale(float textScale)
{
    this->textScale = textScale;
    return *this;
}

lix::Text::PropBuilder& lix::Text::PropBuilder::setAlignment(Alignment alignment)
{
    this->alignment = alignment;
    return *this;
}

lix::Text::PropBuilder& lix::Text::PropBuilder::setLineSpacing(float lineSpacing)
{
    this->lineSpacing = lineSpacing;
    return *this;
}

lix::Text::PropBuilder& lix::Text::PropBuilder::setLetterSpacing(float letterSpacing)
{
    this->letterSpacing = letterSpacing;
    return *this;
}

lix::Text::PropBuilder& lix::Text::PropBuilder::setTextColor(const Color& textColor)
{
    this->textColor = textColor;
    return *this;
}

lix::Text::PropBuilder& lix::Text::PropBuilder::setBorderColor(const Color& borderColor)
{
    this->borderColor = borderColor;
    return *this;
}