#include "objectproc.h"

#include <list>
#include <vector>
#include <unordered_set>
#include <algorithm>

#include "glm/gtc/quaternion.hpp"
#include "gltftypes.h"
#include "json.h"
#include "imageproc.h"
#include "gltfexport.h"

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

void exportSceneDefinition(const gltf::Scene& scene, const std::string& sceneName,
    const fs::path& outputDir,
    std::list<gltf::Texture>& textures, std::list<gltf::Material>& materials,
    std::list<std::vector<char>>& buffers,
     std::list<std::pair<gltf::Buffer,std::pair<size_t, size_t>>>& bufferViews,
    std::list<gltf::Mesh>& meshes,
    std::list<gltf::Node>& nodes, std::list<gltf::Animation>& animations,
    std::list<gltf::Skin>& skins)
{
    const std::string cppFile = (sceneName + ".cpp");
    std::ofstream ofs{outputDir / cppFile};
    const std::string scope = "assets::objects::" + sceneName + "::";
    ofs << "#include \"" << sceneName << ".h\"\n";

    
    int idx{0};
    for(const auto& buf : buffers)
    {
        ofs << "static unsigned char binaryData" << idx << "[] = {\n";
        common::exportBytes((const unsigned char*)buf.data(), buf.size(), ofs);
        ofs << "\n};\n";
    }

    ofs << "const gltf::Buffer " << scope << "buffers[] = {\n";
    std::string delim{""};
    for(const auto& pair : bufferViews)
    {
        const gltf::Buffer& buffer = pair.first;        
        ofs << delim;
        delim = ",\n";
        exportBuffer(buffer, pair.second.first, pair.second.second, ofs, "    ");
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
    /*for(const auto& node : nodes)
    {
        exportNodeChildren(ofs, scope, node);
    }*/
    for(const auto& animation : animations)
    {
        exportAnimation(ofs, scope, animation);
    }
    for(const auto& skin : skins)
    {
        exportSkin(ofs, scope, skin);
    }

    ofs << "static const gltf::Node* " << common::variableName(scene.name) << "_nodes[] = {\n";
    std::string nodeDelim{""};
    for(size_t i{0}; i < scene.nodes_size; ++i)
    {
        ofs << nodeDelim;
        nodeDelim = ",\n";
        ofs << "        &" << scope << nodeName(*scene.nodes[i]);
        //moveme exportNode(ofs, *node, nullptr, scope, "        ");
    };
    ofs << "\n};\n";

    ofs << "const gltf::Scene " << scope << common::variableName(scene.name) << "_scene {\n"
        << "    \"" << scene.name << "\",\n"
        << "    " << common::variableName(scene.name) << "_nodes,\n"
        << "    " << scene.nodes_size << "\n";
    ofs << "};\n";

    ofs.flush();
    ofs.close();
    common::log("Write: " + cppFile);
}

void exportSceneHeader(const gltf::Scene& scene, const std::string& sceneName,
    const fs::path& outputDir,
    std::list<gltf::Texture>& textures, std::list<gltf::Material>& materials,
    std::list<gltf::Mesh>& meshes, std::list<gltf::Node>& nodes,
    std::list<gltf::Animation>& animations,
    std::list<gltf::Skin>& skins)
{
    const std::string headerFile = (sceneName + ".h");
    std::ofstream ofs{outputDir / headerFile};

    ofs << "#pragma once\n"
        << "#include \"gltftypes.h\"\n"
        << "namespace assets {\n"
        << "    namespace objects {\n"
        << "    namespace "
        << sceneName << " {\n";

    ofs << "    extern const gltf::Buffer buffers[];\n";

    for(const auto& tex : textures)
    {
        ofs << "    extern const gltf::Texture " << textureName(tex) << ";\n";
    }
    for(const auto& mat : materials)
    {
        ofs << "    extern const gltf::Material " << materialName(mat) << ";\n";
    }
    for(const auto& mesh : meshes)
    {
        ofs << "    extern const gltf::Mesh " << meshName(mesh) << ";\n";
    }
    for(const auto& node : nodes)
    {
        ofs << "    extern const gltf::Node " << nodeName(node) << ";\n";
    }
    for(const auto& animation : animations)
    {
        ofs << "    extern const gltf::Animation " << animationName(animation) << ";\n";
    }
    for(const auto& skin : skins)
    {
        ofs << "    extern const gltf::Skin " << skinName(skin) << ";\n";
    }
    ofs << "    extern const gltf::Scene " << common::variableName(scene.name) << "_scene" << ";\n";
    ofs << "    }\n    }\n}\n";

    ofs.flush();
    ofs.close();
    common::log("Write: " + headerFile);
}

void loadTextures(const json::Json& obj,
    const fs::path& filePath,
    std::list<gltf::Texture>& textures,
    bool convertToSrgb)
{
    static std::list<std::vector<unsigned char>> textureData;
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
                        f = glm::pow((f + 0.055f) / 1.055f, 2.4f);
                    }
                    data[i] = static_cast<unsigned char>(f * 255.0f);
                }
            }

            std::vector<unsigned char>& texData = textureData.emplace_back(data, data + byteSize);

            textures.push_back({
                imgObj["name"].value(),
                samplerObj["magFilter"].toInt(),
                samplerObj["minFilter"].toInt(),
                width,
                height,
                channels,
                texData.data(),
                texData.size()
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

void loadBuffers(const json::Json& obj, const fs::path& filePath, std::list<std::vector<char>>& buffers, std::list<std::pair<gltf::Buffer,std::pair<size_t, size_t>>>& bufferViews)
{
    for(const auto& bufObj : obj["buffers"])
    {
        std::ifstream binIfs{filePath.parent_path() / bufObj["uri"].value(), std::ios::binary};
        const int byteLength = bufObj["byteLength"].toInt();
        std::vector<char>& buf = buffers.emplace_back(byteLength);
        binIfs.read(buf.data(), byteLength);
        assert(static_cast<size_t>(byteLength) == buf.size());
    }

    size_t i{0};
    for(const auto& acObj : obj["accessors"])
    {
        std::pair<gltf::Buffer, std::pair<size_t, size_t>>& pair = bufferViews.emplace_back();
        gltf::Buffer& buffer = pair.first;
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
        std::list<std::vector<char>>::iterator bufIt = buffers.begin();
        size_t bufferIndex = static_cast<size_t>(bvObj["buffer"].toInt());
        std::advance(bufIt, bufferIndex);

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
        //buffer.data.insert(buffer.data.end(), bufIt->begin() + a, bufIt->begin() + b);
        pair.second.first = bufferIndex;
        pair.second.second = static_cast<size_t>(a);
        buffer.data_size = static_cast<size_t>(numBytes);
        ++i;
    }
}

void loadMeshes(const json::Json& obj, std::list<std::pair<gltf::Buffer,std::pair<size_t, size_t>>>& bufferViews,
    std::list<gltf::Material>& materials, std::list<gltf::Mesh>& meshes)
{
    static std::list<std::vector<gltf::Primitive>> primitiveData;
    static std::list<std::vector<const gltf::Buffer*>> attributesData;
    if(obj.contains("meshes"))
    {
        for(const auto& meshObj : obj["meshes"])
        {
            gltf::Mesh& mesh = meshes.emplace_back();
            mesh.name = meshObj["name"];

            std::vector<gltf::Primitive>& primD = primitiveData.emplace_back();

            for(const auto& primitiveObj : meshObj["primitives"])
            {
                gltf::Primitive& primitive = primD.emplace_back();
                const auto& attribObj = primitiveObj["attributes"];
                static const std::string labels[] = {
                    "POSITION", "NORMAL", "TEXCOORD_0", "JOINTS_0", "WEIGHTS_0"
                };
                std::vector<const gltf::Buffer*>& aData = attributesData.emplace_back();
                for(const std::string& label : labels)
                {
                    if(attribObj.contains(label))
                    {
                        aData.push_back(&elementAt(bufferViews,
                            attribObj[label].toUint()).first);
                    }
                }

                primitive.attributes = aData.data();
                primitive.attributes_size = aData.size();
                
                if(primitiveObj.contains("indices")) {
                    primitive.indices = &elementAt(bufferViews, static_cast<size_t>(
                        primitiveObj["indices"].toInt())).first;
                }
                if(primitiveObj.contains("material")) {
                    primitive.material = &elementAt(materials, static_cast<size_t>(
                        primitiveObj["material"].toInt()));
                }
                else {
                    primitive.material = nullptr;
                }
            }

            mesh.primitives = primD.data();
            mesh.primitives_size = primD.size();
        }
    }
}

void loadAnimations(const json::Json& obj, std::list<std::pair<gltf::Buffer,std::pair<size_t, size_t>>>& bufferViews,
   std::list<gltf::Node>& nodes, std::list<gltf::Animation>& animations)
{
    if(!obj.contains("animations"))
    {
        return;
    }

    static std::list<std::vector<gltf::Channel>> channelData;

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
            sampler.input = &elementAt(bufferViews, static_cast<size_t>(
                        sampObj["input"].toInt())).first;
            sampler.output = &elementAt(bufferViews, static_cast<size_t>(
                        sampObj["output"].toInt())).first;
        }
        std::vector<gltf::Channel>& cData = channelData.emplace_back();
        for(const auto& chanObj : animObj["channels"])
        {
            gltf::Channel& channel = cData.emplace_back();
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
        animation.channels = cData.data();
        animation.channels_size = cData.size();
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
    static std::list<std::vector<const gltf::Node*>> childData;
    for(const auto& node : obj["nodes"])
    {
        if(node.contains("children"))
        {
            std::vector<const gltf::Node*>& cData = childData.emplace_back();
            for(const auto& childObj : node["children"])
            {
                cData.push_back(
                    &elementAt(nodes, childObj.toUint())
                );
                //assert(child->parent == nullptr);
                //gltf::Node& parent = (*nodeIt);
                //child->parent = &parent;
            }
            nodeIt->children = cData.data();
            nodeIt->children_size = cData.size();
        }
        ++nodeIt;
    }
}

void loadSkins(const json::Json& obj, std::list<std::pair<gltf::Buffer,std::pair<size_t, size_t>>>& bufferViews,
    std::list<gltf::Node>& nodes, std::list<gltf::Skin>& skins)
{
    if(!obj.contains("skins"))
    {
        return;
    }

    static std::list<std::vector<const gltf::Node*>> jointsData;

    for(const auto& skinObj : obj["skins"])
    {
        gltf::Skin& skin = skins.emplace_back();

        skin.name = skinObj["name"].value();
        skin.inverseBindMatrices = &elementAt(bufferViews,
            skinObj["inverseBindMatrices"].toUint()).first;
        std::vector<const gltf::Node*>& jData = jointsData.emplace_back();
        for(const auto& jointObj : skinObj["joints"])
        {
            jData.push_back(&elementAt(nodes, jointObj.toUint()));
        }
        skin.joints = jData.data();
        skin.joints_size = jData.size();
    }
}

void procGltf(fs::path filePath, fs::path outputDir, bool convertToSrgb)
{
    json::Json obj;

    std::ifstream ifs{filePath};
    ifs >> obj;

    std::list<gltf::Texture> textures;
    std::list<gltf::Material> materials;
    std::list<std::vector<char>> buffers;
    std::list<std::pair<gltf::Buffer,std::pair<size_t, size_t>>> bufferViews;
    std::list<gltf::Mesh> meshes;
    std::list<gltf::Node> nodes;
    std::list<gltf::Animation> animations;
    std::list<gltf::Skin> skins;

    //std::cout << "loading textures" << std::endl;
    loadTextures(obj, filePath, textures, convertToSrgb);
    //std::cout << "loading materials" << std::endl;
    loadMaterials(obj, textures, materials);
    //std::cout << "loading buffers" << std::endl;
    loadBuffers(obj, filePath, buffers, bufferViews);
    //std::cout << "loading meshes" << std::endl;
    loadMeshes(obj, bufferViews, materials, meshes);
    //std::cout << "loading animations" << std::endl;
    loadNodes(obj, meshes, nodes);
    //std::cout << "loading animations" << std::endl;
    loadAnimations(obj, bufferViews, nodes, animations);
    //std::cout << "loading skins" << std::endl;
    loadSkins(obj, bufferViews, nodes, skins);

    std::list<std::vector<const gltf::Node*>> sceneNodes;

    std::list<gltf::Scene> scenes;
    for(const auto& sceneObj : obj["scenes"])
    {
        gltf::Scene& scene = scenes.emplace_back();
        scene.name = sceneObj["name"].value();
        auto& scNs = sceneNodes.emplace_back();
        for(const auto& nodeId : sceneObj["nodes"])
        {
            /*scene.nodes.push_back(
                &elementAt(nodes, nodeId.toUint())
            );*/
            scNs.push_back(&elementAt(nodes, nodeId.toUint()));
        }
        scene.nodes = scNs.data();
        scene.nodes_size = scNs.size();
    }

    std::string sceneName = filePath.filename().string();
    sceneName = sceneName.substr(0, sceneName.find('.'));

    exportSceneDefinition(scenes.front(), sceneName, outputDir,
        textures, materials, buffers, bufferViews, meshes, nodes, animations, skins);
    exportSceneHeader(scenes.front(), sceneName, outputDir,
        textures, materials, meshes, nodes, animations, skins);
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