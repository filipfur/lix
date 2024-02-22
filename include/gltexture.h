#pragma once

#include <string>
#include "glelement.h"
#include "gltypes.h"
#include "glcolor.h"

namespace lix
{
	class Texture : public Element
	{
	public:
		Texture(unsigned char* bytes, unsigned int width, unsigned int height, GLenum type=GL_UNSIGNED_BYTE,
			GLenum internalFormat=GL_RGBA, GLenum colorFormat=GL_RGBA);
		virtual ~Texture() noexcept;

		static std::shared_ptr<lix::Texture> Basic(lix::Color color=lix::Color::white);
		Texture* setUnpackAlignment(GLuint unpackAlignment=4);
		Texture* setFilter(GLenum min, GLenum mag);
		Texture* setFilter(GLenum filter=GL_NEAREST);
		Texture* setWrap(GLenum wrapS, GLenum wrapT, GLenum wrapR);
		Texture* setWrap(GLenum wrap=GL_CLAMP_TO_EDGE);
		Texture* generateMipmap();
		Texture* setLodBias(float bias);
		Texture* bind(GLuint textureUnit);

		virtual Texture* bind() override;
		virtual Texture* unbind() override;

		void texImage2D(unsigned char* bytes);

		int width() const;
		int height() const;
		GLenum type() const;
		GLenum internalFormat() const;
		GLenum colorFormat() const;

	protected:
		void errorCheck();

	private:
		unsigned int _width;
		unsigned int _height;
		GLenum _type;
		GLenum _internalFormat;
		GLenum _colorFormat;
		static int _active;
		static Texture* _bound[32];
	};

    using TexturePtr = std::shared_ptr<lix::Texture>;
}
