#include "glarrow.h"

#include "primer.h"

std::shared_ptr<lix::Node> lix::arrow(lix::Color color, const glm::vec3& p, const glm::vec3& d)
{
    static float w = 0.04f;
    static float h = 1.0f;
    static std::shared_ptr<lix::VAO> vao{
        new lix::VAO(
            lix::Attributes{lix::Attribute::VEC3},
            {
                {-w, 0.0f, -w},
                {-w, 0.0f, w},
                {w, 0.0f, -w},
                {w, 0.0f, w},
                {0.0f, h, 0.0f}
            },
            {
                0, 1, 4,
                3, 2, 4,
                2, 0, 4,
                1, 3, 4,

                0, 3, 1,
                0, 2, 3
            }
        )
    };
    auto node = std::make_shared<lix::Node>(
        p,
        lix::directionToQuat(d),
        glm::vec3{1.0f, 1.0f, 1.0f}
    );
    node->setMesh(std::make_shared<lix::Mesh>(
        vao,
        std::make_shared<lix::Material>(color)
    ));
    return node;
}