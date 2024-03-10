#include "shaderproc.h"

#include <string>
#include <cstring>

void exportShaderDefinition(const fs::path& outputDir,
    const fs::path& shaderPath,
    const std::string& shaderName,
    const std::string& versionOverride)
{
    std::string shaderCpp{shaderName + ".cpp"};
    std::ofstream ofs{outputDir / shaderCpp};
    std::ifstream ifs{shaderPath};
    ofs << "#include \"" << shaderName << ".h\"\n\n";
    ofs << "const char* assets::shaders::" << shaderName << "{R\"glsl(";
    std::string line;
    if(versionOverride.length() > 0)
    {
        std::getline(ifs, line);
        size_t a = strlen("#version ");
        assert(strncmp(line.c_str(), "#version ", a) == 0);
        ofs << line.substr(0, a) << versionOverride << "\n";
    }
    while(std::getline(ifs, line))
    {
        ofs << line << "\n";
    }
    ifs.close();
    ofs << ")glsl\"};\n";
    common::log("Write: " + shaderCpp);
    ofs.flush();
    ofs.close();
}

void exportShaderDeclaration(const fs::path& outputDir,
    const std::string& shaderName)
{
    const std::string headerFile{shaderName + ".h"};
    std::ofstream ofs{outputDir / headerFile};
    ofs << "#pragma once\n\nnamespace assets {\n    namespace shaders {";
    ofs << "\n        extern const char* " << shaderName << ";";
    ofs << "\n    }\n}";
    common::log("Write: " + headerFile);
    ofs.flush();
    ofs.close();
}

void exportShader(const fs::path& outputDir, const fs::path& shaderPath, const std::string& versionOverride)
{
    const auto shaderFileName = shaderPath.filename();
    const std::string shaderName = common::variableName(shaderFileName.string());
    exportShaderDefinition(outputDir, shaderPath, shaderName, versionOverride);
    exportShaderDeclaration(outputDir, shaderName);
}

void shaderproc::procShaders(fs::path shaderDir, fs::path outputDir, const std::string& versionOverride)
{
    if(fs::is_directory(shaderDir))
    {
        for (const auto& entry : fs::directory_iterator(shaderDir))
        {
            exportShader(outputDir, entry.path(), versionOverride);
        }
    }
    else if(fs::is_regular_file(shaderDir))
    {
        exportShader(outputDir, shaderDir, versionOverride);
    }
}