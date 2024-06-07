#include "plyproc.h"

#include <sstream>
#include "glm/glm.hpp"
#include "gltftypes.h"

#include "gltfexport.h"

namespace {
template <typename T, typename U>
std::vector<T> toVector(const std::vector<U>& v)
{
    size_t numBytes = v.size() * sizeof(U);
    T* buf = (T*)v.data();
    return std::vector<T>(buf, buf + (numBytes / sizeof(T)));
}
}

void procPlyFile(fs::path filePath, fs::path outputDir)
{
    std::ifstream ifs{filePath};
    std::string str;
    static size_t header_n = strlen("end_header");
    static size_t el_vertex_n = strlen("element vertex");
    static size_t el_face_n = strlen("element face");
    bool parse{false};

    int vertexCount{-1};
    int faceCount{-1};

    std::vector<glm::vec3> vertices;
    std::vector<unsigned short> indices;

    while(std::getline(ifs, str))
    {
        if(parse)
        {
            std::istringstream iss(str); 
            if(vertexCount > 0)
            {
                glm::vec3 vert;
                iss >> vert.x >> vert.y >> vert.z;
                vertices.push_back(vert);
                --vertexCount;
            }
            else if(faceCount > 0)
            {
                unsigned short i, a, b, c;
                iss >> i >> a >> b >> c;
                if(i != 3)
                {
                    printf("%s [faceCount=%d]\n", str.c_str(), faceCount);
                }
                assert(i == 3);
                indices.insert(indices.end(), {a, b, c});
                --faceCount;
            }
        }
        else if(strncmp(str.c_str(), "element vertex", el_vertex_n) == 0)
        {
            auto lab = str.substr(str.find_last_of(' ') + 1);
            printf("lab=%s\n", lab.c_str());
            vertexCount = std::stoi(lab);
            vertices.reserve(vertexCount);
            printf("vertexCount=%d\n", vertexCount);
        }
        else if(strncmp(str.c_str(), "element face", el_face_n) == 0)
        {
            auto lab = str.substr(str.find_last_of(' ') + 1);
            printf("lab=%s\n", lab.c_str());
            faceCount = std::stoi(lab);
            indices.reserve(faceCount * 3);
            printf("faceCount=%d\n", vertexCount);
        }
        else if(strncmp(str.c_str(), "end_header", header_n) == 0)
        {
            parse = true;
        }
    }

    std::list<gltf::Buffer> buffers;
    gltf::Buffer& vertexBuffer = buffers.emplace_back();
    vertexBuffer.componentType = 5126;
    vertexBuffer.index = 0;
    vertexBuffer.target = 34962;
    vertexBuffer.type = gltf::Buffer::VEC3;
    vertexBuffer.data = toVector<unsigned char>(vertices);

    gltf::Buffer& indexBuffer = buffers.emplace_back();
    indexBuffer.componentType = 5123;
    indexBuffer.index = 1;
    indexBuffer.target = 34963;
    indexBuffer.type = gltf::Buffer::SCALAR;
    indexBuffer.data = toVector<unsigned char>(indices);

    std::string sceneName = filePath.filename().string();
    sceneName = sceneName.substr(0, sceneName.find('.'));

    gltf::Mesh mesh;
    mesh.name = sceneName;
    gltf::Primitive& primitive = mesh.primitives.emplace_back();
    primitive.attributes.push_back(&vertexBuffer);
    primitive.indices = &indexBuffer;


    { // export .cpp
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

        exportMesh(ofs, scope, mesh);

        ofs.flush();
        ofs.close();

        common::log("Write: " + cppFile);
    }

    { // export .h
        const std::string headerFile = (sceneName + ".h");
        std::ofstream ofs{outputDir / headerFile};

        ofs << "#pragma once\n"
        << "#include \"gltftypes.h\"\n"
        << "namespace assets {\n"
        << "    namespace objects {\n"
        << "    struct "
        << sceneName << " {\n";

        ofs << "    static gltf::Buffer buffers[" << buffers.size() << "];\n";

        ofs << "    static gltf::Mesh " << meshName(mesh) << ";\n";

        ofs << "    };\n    }}\n";

        ofs.flush();
        ofs.close();
        common::log("Write: " + headerFile);
    }
}

void plyproc::procPLY(fs::path inputDir, fs::path outputDir)
{
    if(fs::is_regular_file(inputDir))
    {
        procPlyFile(inputDir, outputDir);
    }
    else if(fs::is_directory(inputDir))
    {
        for(const auto& entry : fs::directory_iterator(inputDir))
        {
            const auto filePath = entry.path();
            if(fs::is_directory(filePath))
            {
                for(const auto& entry2 : fs::directory_iterator(filePath))
                {
                    const auto filePath2 = entry2.path();
                    if(filePath2.extension().string() == ".ply")
                    {
                        procPlyFile(filePath2, outputDir);
                    }
                }
            }
            else if(filePath.extension().string() == ".ply")
            {
                procPlyFile(filePath, outputDir);
            }
        }
    }
}