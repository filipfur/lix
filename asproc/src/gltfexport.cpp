#include "gltfexport.h"

void exportBuffer(const gltf::Buffer& buffer, std::ofstream& ofs, const std::string& tab)
{
    ofs << tab << "{" << buffer.index << ", gltf::Buffer::";
    switch(buffer.type)
    {
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
    ofs << ", " << buffer.target << ", " << buffer.componentType << ", {\n";
    common::exportBytes(buffer.data, ofs);
    ofs << "\n" << tab << "    }\n";
    ofs << tab << "}";
}

void exportVec3(std::ofstream& ofs, const glm::vec3& v)
{
    ofs << '{' << common::floatToString(v.x) << ", "
        << common::floatToString(v.y) << ", "
        << common::floatToString(v.z) << "}";
}

void exportQuat(std::ofstream& ofs, const glm::quat& q)
{
    ofs << '{' << common::floatToString(q.w) << ", "
        << common::floatToString(q.x) << ", "
        << common::floatToString(q.y) << ", "
        << common::floatToString(q.z) << "}";
}

void exportNode(std::ofstream& ofs, const std::string& scope,
    const gltf::Node& node)
{

    ofs << "gltf::Node " << scope << nodeName(node) << "{\""
        << node.name << "\", "
        << (node.mesh ? ("&" + scope + meshName(*node.mesh)) : "nullptr") << ",\n"
        << "    ";
    exportVec3(ofs, node.translation);
    ofs << ",\n    ";
    exportQuat(ofs, node.rotation);
    ofs << ",\n    ";
    exportVec3(ofs, node.scale);
    ofs << ",\n    nullptr, {";
    if(node.children.empty())
    {
        ofs << "}\n";
    }
    else
    {
        std::string nodeDelim{""};
        for(gltf::Node* child : node.children)
        {
            ofs << nodeDelim;
            nodeDelim = ",";
            ofs << "\n        &" << scope << nodeName(*child);
        }
        ofs << "\n    }";
    }
    ofs << "\n};\n";
}

void exportTexture(std::ofstream& ofs, const std::string& scope, const gltf::Texture& texture)
{
    ofs << "gltf::Texture " << scope << textureName(texture) << "{\""
                            << texture.name << "\", "
                            << texture.magFilter << ", "
                            << texture.minFilter << ", " 
                            << texture.width << ", "
                            << texture.height << ", "
                            << texture.channels << ", {\n";
    common::exportBytes(texture.data, ofs);
    ofs << "\n    }\n};\n";
}

void exportMaterial(std::ofstream& ofs, const std::string& scope, const gltf::Material& material)
{
    auto baseTex = material.baseColorTexture;
    ofs << "gltf::Material " << scope << materialName(material) << "{\n"
                        << "    \"" << material.name << "\",\n"
                        << "    glm::vec4{" << material.baseColor.x << ", "
                                            << material.baseColor.y << ", "
                                            << material.baseColor.z << ", "
                                            << material.baseColor.w << "},\n"
                        << "    " << material.metallic << ",\n"
                        << "    " << material.roughness << ",\n"
                        << "    " << (baseTex ? ("&" + scope + textureName(*baseTex)) : "nullptr") << "\n};\n";
}

void exportAnimation(std::ofstream& ofs, const std::string& scope, const gltf::Animation& animation)
{
    ofs << "gltf::Animation " << scope << animationName(animation) << "{"
        << std::quoted(animation.name) << ",\n {\n";
    std::string delim{""};
    for(const auto& channel : animation.channels)
    {
        std::string interstr{"missing"};
        switch(channel.sampler.interpolation)
        {
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
            << "&" << scope << "buffers[" << channel.sampler.input->index << "], "
            << "&" << scope << "buffers[" << channel.sampler.output->index << "]}, "
            << "&" << scope << nodeName(*channel.targetNode) << ", "
            << std::quoted(channel.targetPath) << "}";
    }
    ofs << "    }\n};\n";
}

void exportMesh(std::ofstream& ofs, const std::string& scope, const gltf::Mesh& mesh)
{
    std::string primitiveDelim{""};
    ofs << "gltf::Mesh " << scope  << meshName(mesh) << "{\n"
        << "    \"" << mesh.name << "\",\n"
        << "    {\n";
    
    for(const gltf::Primitive& primitive : mesh.primitives)
    {
        ofs << primitiveDelim << "\n        gltf::Primitive{"
                << (primitive.material ? ("&" + scope + materialName(*primitive.material)) : "nullptr")
                << ", {\n";
        std::string delim{""};
        for(gltf::Buffer* attr : primitive.attributes)
        {
            ofs << delim;
            delim = ",\n";
            ofs << "            &" << scope << "buffers[" << attr->index << "]";
        }
        ofs << "\n        },\n";
        ofs << "        &" << scope << "buffers[" << primitive.indices->index << "]";
        ofs << "\n        }";
        primitiveDelim = ",";
    }
    ofs << "\n    }\n";
    ofs << "};\n";
}

void exportSkin(std::ofstream& ofs, const std::string& scope, const gltf::Skin& skin)
{
    ofs << "gltf::Skin " << scope << skinName(skin) << "{" << std::quoted(skin.name) << ", "
        << "&" << scope << "buffers[" << skin.inverseBindMatrices->index << "], "
        << "{\n";
    std::string delim{""};
    for(gltf::Node* node : skin.joints)
    {
        ofs << delim << "        &" << scope << nodeName(*node);
        delim = ",\n";
    }
    ofs << "\n    }\n};\n";
}