#pragma once

#include "glvertexarray.h"
#include "glmaterial.h"

namespace lix
{
    class Mesh
	{
	public:
		struct Primitive
		{
			Primitive(std::shared_ptr<lix::VertexArray> vao,
				std::shared_ptr<lix::Material> material)
				: vao{vao}, material{material}
			{

			}

			std::shared_ptr<lix::VertexArray> vao;
			std::shared_ptr<lix::Material> material;
		};

		Mesh();
		Mesh(const Mesh& other);
		Mesh(const lix::Attributes& attributes,
			const std::vector<GLfloat>& vertices,
			GLenum mode=GL_TRIANGLES,
            GLenum usage=GL_STATIC_DRAW,
			std::shared_ptr<lix::Material> material=nullptr);
		Mesh(const lix::Attributes& attributes,
			const std::vector<GLfloat>& vertices, const std::vector<GLuint>& indices,
            GLenum mode=GL_TRIANGLES,
            GLenum usage=GL_STATIC_DRAW,
			std::shared_ptr<lix::Material> material=nullptr);

		Mesh& operator=(const Mesh& other) = delete;

		virtual ~Mesh() noexcept;

		Mesh* clone() const;

		const lix::Mesh::Primitive& createPrimitive(GLenum mode=GL_TRIANGLES,
			std::shared_ptr<lix::Material> material=nullptr);

		const Primitive& primitive(size_t index=0) const;

		std::shared_ptr<lix::Material> material(size_t index=0) const;

		std::shared_ptr<lix::VertexArray> vertexArray(size_t index=0) const;

		size_t count() const;

		std::shared_ptr<VertexArray> bindVertexArray(size_t index);

		void draw(size_t index);

		void draw();

	private:
		static const lix::Material defaultMaterial;
		std::vector<Primitive> _primitives;
	};

	using MeshPtr = std::shared_ptr<Mesh>;
}