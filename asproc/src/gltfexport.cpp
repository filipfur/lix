#include "gltfexport.h"

void exportBuffer(const gltf::Buffer &buffer, size_t bufferIndex, size_t offset,
                  std::ofstream &ofs, const std::string &tab) {
    ofs << tab << "{" << buffer.index << ", gltf::Buffer::";
    switch (buffer.type) {
    case gltf::Buffer::SCALAR:
        ofs << "SCALAR";
        break;
    case gltf::Buffer::VEC2:
        ofs << "VEC2";
        break;
    case gltf::Buffer::VEC3:
        ofs << "VEC3";
        break;
    case gltf::Buffer::VEC4:
        ofs << "VEC4";
        break;
    case gltf::Buffer::MAT4:
        ofs << "MAT4";
        break;
    }
    ofs << ", " << buffer.target << ", " << buffer.componentType << ", \n";
    // common::exportBytes(buffer.data, buffer.data_size, ofs);
    ofs << "binaryData" << bufferIndex << " + " << offset << ",";
    ofs << "\n" << tab << buffer.data_size << " \n";
    ofs << tab << "}";
}

void exportVec3(std::ofstream &ofs, const glm::vec3 &v) {
    ofs << '{' << common::floatToString(v.x) << ", "
        << common::floatToString(v.y) << ", " << common::floatToString(v.z)
        << "}";
}

void exportQuat(std::ofstream &ofs, const glm::quat &q) {
    ofs << '{' << common::floatToString(q.w) << ", "
        << common::floatToString(q.x) << ", " << common::floatToString(q.y)
        << ", " << common::floatToString(q.z) << "}";
}

void exportNode(std::ofstream &ofs, const std::string &scope,
                const gltf::Node &node) {
    /*if(node.children_size > 0)
    {
        ofs << "static const gltf::Node* " << nodeName(node) <<
    "_children[];\n";
    }*/

    exportNodeChildren(ofs, scope, node);

    ofs << "const gltf::Node " << scope << nodeName(node) << "{\"" << node.name
        << "\", "
        << (node.mesh ? ("&" + scope + meshName(*node.mesh)) : "nullptr")
        << ",\n"
        << "    ";
    exportVec3(ofs, node.translation);
    ofs << ",\n    ";
    exportQuat(ofs, node.rotation);
    ofs << ",\n    ";
    exportVec3(ofs, node.scale);
    ofs << ",\n    nullptr, ";
    if (node.children_size == 0) {
        ofs << "nullptr, 0";
    } else {
        ofs << nodeName(node) << "_children, " << node.children_size;
    }
    ofs << "\n};\n";
}

void exportNodeChildren(std::ofstream &ofs, const std::string &scope,
                        const gltf::Node &node) {
    if (node.children_size < 1) {
        return;
    }
    ofs << "static const gltf::Node* " << nodeName(node) << "_children[] = {\n";
    std::string nodeDelim{""};
    // for(const gltf::Node* child : node.children)
    for (size_t i{0}; i < node.children_size; ++i) {
        ofs << nodeDelim;
        nodeDelim = ",\n";
        ofs << "        &" << scope << nodeName(*node.children[i]);
    }
    ofs << "\n};\n";
}

void exportTexture(std::ofstream &ofs, const std::string &scope,
                   const gltf::Texture &texture) {
    ofs << "static const unsigned char " << textureName(texture)
        << "_data[] = {\n";
    common::exportBytes(texture.data, texture.data_size, ofs);
    ofs << "\n};\n";
    ofs << "const gltf::Texture " << scope << textureName(texture) << "{\""
        << texture.name << "\", " << texture.magFilter << ", "
        << texture.minFilter << ", " << texture.width << ", " << texture.height
        << ", " << texture.channels << ", " << textureName(texture)
        << "_data};\n";
}

void exportMaterial(std::ofstream &ofs, const std::string &scope,
                    const gltf::Material &material) {
    auto baseTex = material.baseColorTexture;
    ofs << "const gltf::Material " << scope << materialName(material) << "{\n"
        << "    \"" << material.name << "\",\n"
        << "    glm::vec4{" << material.baseColor.x << ", "
        << material.baseColor.y << ", " << material.baseColor.z << ", "
        << material.baseColor.w << "},\n"
        << "    " << material.metallic << ",\n"
        << "    " << material.roughness << ",\n"
        << "    "
        << (baseTex ? ("&" + scope + textureName(*baseTex)) : "nullptr")
        << "\n};\n";
}

void exportAnimation(std::ofstream &ofs, const std::string &scope,
                     const gltf::Animation &animation) {
    ofs << "const gltf::Channel " << animationName(animation)
        << "_channels[] = {\n";
    std::string delim{""};
    // for(const auto& channel : animation.channels)
    for (size_t i{0}; i < animation.channels_size; ++i) {
        const gltf::Channel &channel = animation.channels[i];
        std::string interstr{"missing"};
        switch (channel.sampler.interpolation) {
        case gltf::Sampler::STEP:
            interstr = "STEP";
            break;
        case gltf::Sampler::LINEAR:
            interstr = "LINEAR";
            break;
        }
        ofs << delim;
        delim = ",\n";
        ofs << "        {{gltf::Sampler::" << interstr << ", "
            << "&" << scope << "buffers[" << channel.sampler.input->index
            << "], "
            << "&" << scope << "buffers[" << channel.sampler.output->index
            << "]}, "
            << "&" << scope << nodeName(*channel.targetNode) << ", "
            << std::quoted(channel.targetPath) << "}";
    }
    ofs << "\n};\n";
    ofs << "const gltf::Animation " << scope << animationName(animation) << "{"
        << std::quoted(animation.name) << ", " << animationName(animation)
        << "_channels, " << animation.channels_size << " \n};\n";
}

void exportMesh(std::ofstream &ofs, const std::string &scope,
                const gltf::Mesh &mesh) {
    std::string primitiveDelim{""};

    // for(const gltf::Primitive& primitive : mesh.primitives)

    for (size_t i{0}; i < mesh.primitives_size; ++i) {
        const gltf::Primitive &primitive = mesh.primitives[i];
        ofs << "static const gltf::Buffer* " << meshName(mesh) << "_prim" << i
            << "_attr" << "[] = {\n";
        std::string delim{""};
        for (size_t j{0}; j < primitive.attributes_size; ++j) {
            ofs << delim;
            delim = ",\n";
            ofs << "&" << scope << "buffers[" << primitive.attributes[j]->index
                << "]";
            // const gltf::Buffer& buffer = primitive.attributes[j];
            // exportBuffer(buffer, 0, 0, ofs, "    "); //TODO: FIXME
        }
        ofs << "\n};\n";
    }

    ofs << "static const gltf::Primitive " << meshName(mesh) << "_primitives"
        << "[] = {";

    for (size_t i{0}; i < mesh.primitives_size; ++i) {
        const gltf::Primitive &primitive = mesh.primitives[i];
        ofs << primitiveDelim << "\n    {\n        "
            << (primitive.material
                    ? ("&" + scope + materialName(*primitive.material))
                    : "nullptr")
            << ",\n";
        ofs << "        " << meshName(mesh) << "_prim" << i << "_attr, "
            << primitive.attributes_size << ", \n";
        ofs << "        &" << scope << "buffers[" << primitive.indices->index
            << "]";
        ofs << "\n    }";
        primitiveDelim = ",";
    }
    ofs << "\n};\n";

    ofs << "const gltf::Mesh " << scope << meshName(mesh) << "{\n"
        << "    \"" << mesh.name << "\",\n"
        << "    " << meshName(mesh) << "_primitives,\n"
        << "    " << mesh.primitives_size << "\n};\n";
}

void exportSkin(std::ofstream &ofs, const std::string &scope,
                const gltf::Skin &skin) {
    ofs << "static const gltf::Node* " << skinName(skin) << "_joints[] = {\n";
    std::string delim{""};
    for (size_t i{0}; i < skin.joints_size; ++i) {
        ofs << delim << "        &" << scope << nodeName(*skin.joints[i]);
        delim = ",\n";
    }
    ofs << "\n};\n";

    ofs << "const gltf::Skin " << scope << skinName(skin) << "{"
        << std::quoted(skin.name) << ", "
        << "&" << scope << "buffers[" << skin.inverseBindMatrices->index
        << "], " << skinName(skin) << "_joints, " << skin.joints_size
        << "\n};\n";
}