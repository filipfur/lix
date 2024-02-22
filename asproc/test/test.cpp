#include <iostream>
#include <cstring>

#include "gen/shaders/object_color_vert.h"
#include "gen/objects/cube.h"

int GL_FLO4T = 5050;

namespace assets::models {

    extern gltf::Material xmas_material;
    extern gltf::Mesh xmas_mesh;
    extern gltf::Scene xmas_scene;
}

int main(int argc, const char* argv[])
{
    std::cout << strlen(assets::shaders::object_color_vert) << std::endl;

    for(gltf::Node* node : assets::models::xmas_scene.nodes)
    {
        std::cout << "node->name=" << node->name << std::endl;
    }

    //std::cout << ">>:" << assets::objects::cube::Cube_mesh.name << std::endl;

    auto cube = new assets::objects::cube();

    std::cout << "==:" << cube->Cube_mesh.name << std::endl;

    return 0;
}


gltf::Material assets::models::xmas_material {
    "Material.001",
    glm::vec4{1.0f, 0.0f, 0.0f, 1.0f},
    0.5f,
    0.0f
};

static gltf::Buffer buffers[3] = {
    gltf::Buffer {
        0,
        gltf::Buffer::VEC3,
        GL_FLO4T,
        5011,
        {
            0xFF, 0xFF, 0xFF
        }
    },
    gltf::Buffer {
        1,
        gltf::Buffer::VEC2,
        GL_FLO4T,
        5011,
        {
            0xFF, 0xFF, 0xFF
        }
    },
    gltf::Buffer {
        2,
        gltf::Buffer::SCALAR,
        GL_FLO4T,
        5011,
        {
            0xFF, 0xFF, 0xFF
        }
    }
};

gltf::Mesh assets::models::xmas_mesh {
    "xmas",
    {
        gltf::Primitive{
            &assets::models::xmas_material,
            {
                &buffers[0],
                &buffers[1]
            },
            &buffers[2]
        }
    }
};

static gltf::Node rudolfNode{"Rudolf", &assets::models::xmas_mesh};

static gltf::Node prancerNode{"Prancer", &assets::models::xmas_mesh};

gltf::Scene assets::models::xmas_scene {"Scene", {
    &rudolfNode,
    &prancerNode
}};