#include "objectproc.h"

#include <list>
#include <unordered_set>
#include <algorithm>

#include "gtc/quaternion.hpp"
#include "gltftypes.h"
#include "json.h"
#include "imageproc.h"

namespace
{
    template <typename T>
    T& elementAt(std::list<T>& l, size_t index)
    {
        auto it = l.begin();
        std::advance(it, index);
        return *it;
    }
}

inline std::string textureName(const gltf::Texture& texture)
{
    return common::variableName(texture.name) + "_tex";
}

inline std::string materialName(const gltf::Material& material)
{
    return common::variableName(material.name) + "_mat";
}

inline std::string meshName(const gltf::Mesh& mesh)
{
    return common::variableName(mesh.name) + "_mesh";
}

inline std::string nodeName(const gltf::Node& node)
{
    return common::variableName(node.name) + "_node";
}

inline std::string sceneName(const gltf::Scene& scene)
{
    return common::variableName(scene.name) + "_scene";
}

inline std::string animationName(const gltf::Animation& animation)
{
    return common::variableName(animation.name) + "_anim";
}

inline std::string skinName(const gltf::Skin& skin)
{
    return common::variableName(skin.name) + "_skin";
}

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

void exportSceneHeader(const gltf::Scene& scene, const std::string& sceneName,
    const fs::path& outputDir,
    std::list<gltf::Texture>& textures, std::list<gltf::Material>& materials,
    std::list<gltf::Buffer>& buffers, std::list<gltf::Mesh>& meshes,
    std::list<gltf::Node>& nodes, std::list<gltf::Animation>& animations,
    std::list<gltf::Skin>& skins)
{
    const std::string headerFile = (sceneName + ".h");
    std::ofstream ofs{outputDir / headerFile};

    ofs << "#pragma once\n"
        << "#include \"gltftypes.h\"\n"
        << "namespace assets {\n"
        << "    namespace objects {\n"
        << "    struct "
        << sceneName << " {\n";

    ofs << "    static gltf::Buffer buffers[" << buffers.size() << "];\n";

    for(const auto& tex : textures)
    {
        ofs << "    static gltf::Texture " << textureName(tex) << ";\n";
    }
    for(const auto& mat : materials)
    {
        ofs << "    static gltf::Material " << materialName(mat) << ";\n";
    }
    for(const auto& mesh : meshes)
    {
        ofs << "    static gltf::Mesh " << meshName(mesh) << ";\n";
    }
    for(const auto& node : nodes)
    {
        ofs << "    static gltf::Node " << nodeName(node) << ";\n";
    }
    for(const auto& animation : animations)
    {
        ofs << "    static gltf::Animation " << animationName(animation) << ";\n";
    }
    for(const auto& skin : skins)
    {
        ofs << "    static gltf::Skin " << skinName(skin) << ";\n";
    }
    ofs << "    static gltf::Scene " << common::variableName(scene.name) << "_scene" << ";\n";
    ofs << "    };\n    }}\n";

    ofs.flush();
    ofs.close();
    common::log("Write: " + headerFile);
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

void exportSceneDefinition(const gltf::Scene& scene, const std::string& sceneName,
    const fs::path& outputDir,
    std::list<gltf::Texture>& textures, std::list<gltf::Material>& materials,
    std::list<gltf::Buffer>& buffers, std::list<gltf::Mesh>& meshes,
    std::list<gltf::Node>& nodes, std::list<gltf::Animation>& animations,
    std::list<gltf::Skin>& skins)
{
    const std::string cppFile = (sceneName + ".cpp");
    std::ofstream ofs{outputDir / cppFile};
    const std::string scope = "assets::objects::" + sceneName + "::";
    ofs << "#include \"" << sceneName << ".h\"\n";

    ofs << "gltf::Buffer " << scope << "buffers[" << buffers.size() << "] = {\n";
    std::string delim{""};
    for(const auto& buffer : buffers)
    {
        ofs << delim;
        delim = ",\n";
        exportBuffer(buffer, ofs, "    ");
    }
    ofs << "\n};\n";
    for(const auto& tex : textures)
    {
        exportTexture(ofs, scope, tex);
    }
    for(const auto& mat : materials)
    {
        exportMaterial(ofs, scope, mat);
    }
    for(const auto& mesh : meshes)
    {
        exportMesh(ofs, scope, mesh);
    }
    for(const auto& node : nodes)
    {
        exportNode(ofs, scope, node);
    }
    for(const auto& animation : animations)
    {
        exportAnimation(ofs, scope, animation);
    }
    for(const auto& skin : skins)
    {
        exportSkin(ofs, scope, skin);
    }

    ofs << "gltf::Scene " << scope << common::variableName(scene.name) << "_scene {\n"
        << "    \"" << scene.name << "\",\n"
        << "    {\n";
    std::string nodeDelim{""};
    for(gltf::Node* node : scene.nodes)
    {
        ofs << nodeDelim;
        nodeDelim = ",\n";
        ofs << "        &" << scope << nodeName(*node);
        //moveme exportNode(ofs, *node, nullptr, scope, "        ");
    };
    ofs << "\n    }\n";
    ofs << "};\n";

    ofs.flush();
    ofs.close();
    common::log("Write: " + cppFile);
}

void loadTextures(const json::Json& obj,
    const fs::path& filePath,
    std::list<gltf::Texture>& textures,
    bool convertToSrgb)
{
    if(obj.contains("textures"))
    {
        for(const auto& texObj : obj["textures"])
        {
            size_t source = texObj["source"].toUint();
            size_t sampler = texObj["sampler"].toUint();
            const auto& imgObj = obj["images"].at(source);
            const auto& samplerObj = obj["samplers"].at(sampler);

            int width;
            int height;
            int channels;
            unsigned char* data = imageproc::loadImage(
                filePath.parent_path() / imgObj["uri"].value(),
                false,
                width,
                height,
                channels
            );

            int byteSize{width * height * channels};

            if(convertToSrgb)
            {
                for(int i{0}; i < byteSize; ++i)
                {
                    float f = static_cast<float>(data[i]) / 255.0f;
                    if (f <= 0.04045f) {
                        f = f / 12.92f;
                    } else {
                        f = pow((f + 0.055f) / 1.055f, 2.4f);
                    }
                    data[i] = static_cast<unsigned char>(f * 255.0f);
                }
            }

            textures.push_back({
                imgObj["name"].value(),
                samplerObj["magFilter"].toInt(),
                samplerObj["minFilter"].toInt(),
                width,
                height,
                channels,
                std::vector<unsigned char>(data, data + byteSize)
            });
            imageproc::freeImage(data);
        }
    }
}

void loadMaterials(const json::Json& obj, std::list<gltf::Texture>& textures,
    std::list<gltf::Material>& materials)
{
    if(obj.contains("materials"))
    {
        for(const auto& materialObj : obj["materials"])
        {
            gltf::Material& material = materials.emplace_back();
            material.name = materialObj["name"].value();
            if(materialObj.contains("pbrMetallicRoughness"))
            {
                const auto& pbrObj = materialObj["pbrMetallicRoughness"];
                if(pbrObj.contains("baseColorFactor"))
                {
                    const auto& bcf = pbrObj["baseColorFactor"];
                    material.baseColor[0] = bcf[0].toFloat();
                    material.baseColor[1] = bcf[1].toFloat();
                    material.baseColor[2] = bcf[2].toFloat();
                    material.baseColor[3] = bcf[3].toFloat();
                }
                else
                {
                    material.baseColor = glm::vec4{1.0f};
                }
                if(pbrObj.contains("metallicFactor"))
                {
                    material.metallic = pbrObj["metallicFactor"].toFloat();
                }
                else
                {
                    material.metallic = 0.0f;
                }
                if(pbrObj.contains("roughnessFactor"))
                {
                    material.roughness = pbrObj["roughnessFactor"].toFloat();
                }
                else
                {
                    material.roughness = 0.5f;
                }
                if(pbrObj.contains("baseColorTexture"))
                {
                    int index = pbrObj["baseColorTexture"]["index"].toInt();
                    material.baseColorTexture = &elementAt(textures, static_cast<size_t>(index));
                }
                else
                {
                    material.baseColorTexture = nullptr;
                }
            }
        }
    }
}

void loadBuffers(const json::Json& obj, const fs::path& filePath, std::list<gltf::Buffer>& buffers)
{
    std::list<std::vector<char>> bufs;
    for(const auto& bufObj : obj["buffers"])
    {
        std::ifstream binIfs{filePath.parent_path() / bufObj["uri"].value(), std::ios::binary};
        const int byteLength = bufObj["byteLength"].toInt();
        std::vector<char>& buf = bufs.emplace_back(byteLength);
        binIfs.read(buf.data(), byteLength);
        assert(static_cast<size_t>(byteLength) == buf.size());
    }

    size_t i{0};
    for(const auto& acObj : obj["accessors"])
    {
        gltf::Buffer& buffer = buffers.emplace_back();
        buffer.index = i;
        const auto& bvObj = obj["bufferViews"][i];
        assert(acObj["bufferView"].toUint() == i);
        if(bvObj.contains("target"))
        {
            buffer.target = bvObj["target"].toInt();
        }
        else
        {
            buffer.target = 0;
        }
        std::list<std::vector<char>>::iterator bufIt = bufs.begin();
        std::advance(bufIt, bvObj["buffer"].toInt());

        buffer.componentType = acObj["componentType"].toInt();

        const std::string acType = acObj["type"].value();
        if(acType == "SCALAR")
        {
            buffer.type = gltf::Buffer::SCALAR;
        }
        else if(acType == "VEC2")
        {
            buffer.type = gltf::Buffer::VEC2;
        }
        else if(acType == "VEC3")
        {
            buffer.type = gltf::Buffer::VEC3;
        }
        else if(acType == "VEC4")
        {
            buffer.type = gltf::Buffer::VEC4;
        }
        else if(acType == "MAT4")
        {
            buffer.type = gltf::Buffer::MAT4;
        }
        else
        {
            throw std::runtime_error("objectproc.cpp: unsupported acType");
        }

        int a = bvObj["byteOffset"].toInt();
        int numBytes = bvObj["byteLength"].toInt();
        int b = a + numBytes;
        buffer.data.insert(buffer.data.end(), bufIt->begin() + a, bufIt->begin() + b);
        ++i;
    }
}

void copyBuffer(gltf::Buffer& a, const gltf::Buffer& b)
{
    a.componentType = b.componentType;
    a.data.reserve(b.data.size());
    a.data.insert(a.data.end(), b.data.begin(), b.data.end());
    a.target = b.target;
    a.type = b.type;
}

void loadMeshes(const json::Json& obj, std::list<gltf::Buffer>& buffers,
    std::list<gltf::Material>& materials, std::list<gltf::Mesh>& meshes)
{
    if(obj.contains("meshes"))
    {
        for(const auto& meshObj : obj["meshes"])
        {
            gltf::Mesh& mesh = meshes.emplace_back();
            mesh.name = meshObj["name"];

            for(const auto& primitiveObj : meshObj["primitives"])
            {
                gltf::Primitive& primitive = mesh.primitives.emplace_back();
                const auto& attribObj = primitiveObj["attributes"];
                static const std::string labels[] = {
                    "POSITION", "NORMAL", "TEXCOORD_0", "JOINTS_0", "WEIGHTS_0"
                };
                for(const std::string& label : labels)
                {
                    if(attribObj.contains(label))
                    {
                        primitive.attributes.push_back(&elementAt(buffers,
                            attribObj[label].toUint()));
                    }
                }
                
                if(primitiveObj.contains("indices")) {
                    primitive.indices = &elementAt(buffers, static_cast<size_t>(
                        primitiveObj["indices"].toInt()));
                }
                if(primitiveObj.contains("material")) {
                    primitive.material = &elementAt(materials, static_cast<size_t>(
                        primitiveObj["material"].toInt()));
                }
                else {
                    primitive.material = nullptr;
                }
            }
        }
    }
}

void loadAnimations(const json::Json& obj, std::list<gltf::Buffer>& buffers,
   std::list<gltf::Node>& nodes, std::list<gltf::Animation>& animations)
{
    if(!obj.contains("animations"))
    {
        return;
    }
    for(const auto& animObj : obj["animations"])
    {
        gltf::Animation& animation = animations.emplace_back();
        animation.name = animObj["name"].value();
        std::list<gltf::Sampler> samplers;
        for(const auto& sampObj : animObj["samplers"])
        {
            gltf::Sampler::Interpolation interp = gltf::Sampler::LINEAR;
            if(sampObj.contains("interpolation"))
            {
                const std::string intstr = sampObj["interpolation"].value();
                if(intstr == "STEP")
                {
                    interp = gltf::Sampler::STEP;
                }
                else if(intstr == "LINEAR")
                {
                    interp = gltf::Sampler::LINEAR;
                }
            }
            gltf::Sampler& sampler = samplers.emplace_back();
            sampler.interpolation = interp;
            sampler.input = &elementAt(buffers, static_cast<size_t>(
                        sampObj["input"].toInt()));
            sampler.output = &elementAt(buffers, static_cast<size_t>(
                        sampObj["output"].toInt()));
        }
        for(const auto& chanObj : animObj["channels"])
        {
            gltf::Channel& channel = animation.channels.emplace_back();
            gltf::Sampler& sampler = elementAt(samplers,
                static_cast<size_t>(chanObj["sampler"].toInt()));
            channel.sampler.interpolation = sampler.interpolation;
            channel.sampler.input = sampler.input;
            channel.sampler.output = sampler.output;
            channel.targetNode = &elementAt(nodes, 
                static_cast<size_t>(chanObj["target"]["node"].toInt())
            );
            channel.targetPath = chanObj["target"]["path"].value();
        }
    }
}

void loadNodes(const json::Json& obj, std::list<gltf::Mesh>& meshes, std::list<gltf::Node>& nodes)
{
    if(!obj.contains("nodes"))
    {
        return;
    }
    for(const auto& nodeObj : obj["nodes"])
    {
        gltf::Node& node = nodes.emplace_back();
        node.name = nodeObj["name"].value();
        if(nodeObj.contains("translation"))
        {
            const auto& t = nodeObj["translation"];
            node.translation.x = t.at(0).toFloat();
            node.translation.y = t.at(1).toFloat();
            node.translation.z = t.at(2).toFloat();
        }
        else
        {
            node.translation = glm::vec3{0.0f};
        }
        if(nodeObj.contains("rotation"))
        {
            const auto& r = nodeObj["rotation"];
            node.rotation.x = r.at(0).toFloat();
            node.rotation.y = r.at(1).toFloat();
            node.rotation.z = r.at(2).toFloat();
            node.rotation.w = r.at(3).toFloat();
        }
        else
        {
            node.rotation = glm::quat{1.0f, 0.0f, 0.0f, 0.0f};
        }
        if(nodeObj.contains("scale"))
        {
            const auto& s = nodeObj["scale"];
            node.scale.x = s.at(0).toFloat();
            node.scale.y = s.at(1).toFloat();
            node.scale.z = s.at(2).toFloat();
        }
        else
        {
            node.scale = glm::vec3{1.0f};
        }
        node.mesh = nullptr;
        if(nodeObj.contains("mesh"))
        {
            node.mesh = &elementAt(meshes, static_cast<size_t>(
                nodeObj["mesh"].toInt())
            );
        }
        node.parent = nullptr;
    }
    auto nodeIt = nodes.begin();
    for(const auto& node : obj["nodes"])
    {
        if(node.contains("children"))
        {
            for(const auto& childObj : node["children"])
            {
                gltf::Node* child = &elementAt(nodes, childObj.toUint());
                nodeIt->children.push_back(
                    child
                );
                assert(child->parent == nullptr);
                gltf::Node& parent = (*nodeIt);
                child->parent = &parent;
            }
        }
        ++nodeIt;
    }
}

void loadSkins(const json::Json& obj, std::list<gltf::Buffer>& buffers,
    std::list<gltf::Node>& nodes, std::list<gltf::Skin>& skins)
{
    if(!obj.contains("skins"))
    {
        return;
    }
    for(const auto& skinObj : obj["skins"])
    {
        gltf::Skin& skin = skins.emplace_back();

        skin.name = skinObj["name"].value();
        skin.inverseBindMatrices = &elementAt(buffers,
            skinObj["inverseBindMatrices"].toUint());
        for(const auto& jointObj : skinObj["joints"])
        {
            skin.joints.push_back(&elementAt(nodes, jointObj.toUint()));
        }
    }
}

void procGltf(fs::path filePath, fs::path outputDir, bool convertToSrgb)
{
    json::Json obj;

    std::ifstream ifs{filePath};
    ifs >> obj;

    std::list<gltf::Texture> textures;
    std::list<gltf::Material> materials;
    std::list<gltf::Buffer> buffers;
    std::list<gltf::Mesh> meshes;
    std::list<gltf::Node> nodes;
    std::list<gltf::Animation> animations;
    std::list<gltf::Skin> skins;

    //std::cout << "loading textures" << std::endl;
    loadTextures(obj, filePath, textures, convertToSrgb);
    //std::cout << "loading materials" << std::endl;
    loadMaterials(obj, textures, materials);
    //std::cout << "loading buffers" << std::endl;
    loadBuffers(obj, filePath, buffers);
    //std::cout << "loading meshes" << std::endl;
    loadMeshes(obj, buffers, materials, meshes);
    //std::cout << "loading animations" << std::endl;
    loadNodes(obj, meshes, nodes);
    //std::cout << "loading animations" << std::endl;
    loadAnimations(obj, buffers, nodes, animations);
    //std::cout << "loading skins" << std::endl;
    loadSkins(obj, buffers, nodes, skins);

    std::list<gltf::Scene> scenes;
    for(const auto& sceneObj : obj["scenes"])
    {
        gltf::Scene& scene = scenes.emplace_back();
        scene.name = sceneObj["name"].value();
        for(const auto& nodeId : sceneObj["nodes"])
        {
            scene.nodes.push_back(
                &elementAt(nodes, nodeId.toUint())
            );
        }
    }

    std::string sceneName = filePath.filename().string();
    sceneName = sceneName.substr(0, sceneName.find('.'));

    exportSceneDefinition(scenes.front(), sceneName, outputDir,
        textures, materials, buffers, meshes, nodes, animations, skins);
    exportSceneHeader(scenes.front(), sceneName, outputDir,
        textures, materials, buffers, meshes, nodes, animations, skins);
}

void objectproc::procObject(fs::path inputDir, fs::path outputDir, bool convertToSrgb)
{
    for(const auto& entry : fs::directory_iterator(inputDir))
    {
        const auto filePath = entry.path();
        if(fs::is_directory(filePath))
        {
            for(const auto& entry2 : fs::directory_iterator(filePath))
            {
                const auto filePath2 = entry2.path();
                if(filePath2.extension().string() == ".gltf")
                {
                    procGltf(filePath2, outputDir, convertToSrgb);
                }
            }
        }
        else if(filePath.extension().string() == ".gltf")
        {
            procGltf(filePath, outputDir, convertToSrgb);
        }
    }
}