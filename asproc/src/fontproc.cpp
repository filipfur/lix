#include "fontproc.h"

#include "imageproc.h"

#include "json.h"

void exportFontDeclaration(std::ofstream& ofs, const std::string& fontName)
{
    ofs << "#pragma once\n"
        << "#include <memory>\n"
        << "#include \"gltftypes.h\"\n"
        << "#include \"ttf.h\"\n"
        << "namespace assets {\n"
        << "    namespace fonts {\n"
        << "        namespace " << fontName << " {\n"
        << "            std::shared_ptr<ttf::Font> create();\n";
//        << "        struct " << fontName << " {\n";
//    ofs << "            static ttf::Font font;\n";
//    ofs << "            static unsigned char imageData[];\n";
//    ofs << "            static unsigned int imageWidth;\n";
//    ofs << "            static unsigned int imageHeight;\n";
//    ofs << "            static unsigned int imageChannels;\n";
    ofs << "        }\n    }\n}\n";
    common::log("Write: " + fontName + ".h");
}

std::string charToString(char c)
{
    std::string str{"'"};
    if(c == '\'' || c == '\\')
    {
        str += '\\';
    }
    str += c;
    str += "'";
    return str;
}

void exportJsonFont(std::ofstream& ofs, const fs::path& fontPath, const std::string& /*scope*/)
{
    json::Json obj;
    std::ifstream ifs{fontPath};
    ifs >> obj;

    ofs << std::quoted(obj["name"].value()) << ",\n"
    << "    " << obj["size"].toInt() << ", " << obj["width"].toInt() << ", " << obj["height"].toInt() << ",{\n";
    std::string delim{""};
    for(const auto& chObj : obj["characters"].children())
    {
        const std::string token = charToString(chObj.first.at(0));
        const auto& c = chObj.second;
        ofs << delim << "        {" << token << ", {" << token << ", "
        << c["x"].toInt() << ".0f" << ", "
        << c["y"].toInt() << ".0f" << ", "
        << c["width"].toInt() << ".0f" << ", "
        << c["height"].toInt() << ".0f" << ", "
        << c["originX"].toInt() << ".0f" << ", "
        << c["originY"].toInt() << ".0f" << ", "
        << c["advance"].toInt() << ".0f"
        << "}}";
        delim = ",\n";
    }
    ofs << "\n    }";
}

void exportFontDefinition(std::ofstream& ofs, const std::string& fontName,
    const fs::path& fontPath, const fs::path& imagePath, bool flipY)
{
    int width;
    int height;
    int channels;
    const std::string scope = "assets::fonts::" + fontName;
    unsigned char* data = imageproc::loadImage(imagePath, flipY, width, height, channels);
    ofs << "#include \"" << fontName << ".h\"\n";

    ofs << "std::shared_ptr<ttf::Font> " << scope << "::create() {\n"
        << "    auto font = std::shared_ptr<ttf::Font>(new ttf::Font{";
    exportJsonFont(ofs, fontPath, scope);
    int numBytes = width * height * channels;
    ofs << ", {";
    int pixelWidth = width * channels;
    std::ios_base::fmtflags f( ofs.flags() );
    for(int b{0}; b < numBytes - 1; ++b)
    {
        if(b % pixelWidth == 0) { ofs << "\n  "; }
        ofs << "0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(data[b]) << ',';
    }
    ofs << "0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(data[numBytes - 1]);
    ofs.flags( f );
    ofs << "\n},\n";

    ofs << "    " << width << ", " << height << ", " << channels << "});\n"
        << "    return font;\n}\n";
    //ofs << "unsigned int " << scope << "::imageWidth{" << width << "};\n";
    //ofs << "unsigned int " << scope << "::imageHeight{" << height << "};\n"; 
    //ofs << "unsigned int " << scope << "::imageChannels{" << channels << "};\n"; 

    imageproc::freeImage(data);

    common::log("Write: " + fontName + ".cpp");
}

void exportFont(const fs::path& outputDir, const fs::path& fontPath, bool flipY)
{
    std::string fontName = fontPath.filename().string();
    fontName = common::variableName(fontName.substr(0, fontName.rfind('.')));

    const fs::path imagePath{fontPath.parent_path() / (fontName + ".png")};
    std::cout << "outputDir=" << outputDir << std::endl;
    std::cout << "imagePath=" << imagePath << std::endl;

    std::ofstream ofsDec{outputDir / (fontName + ".h")};
    exportFontDeclaration(ofsDec, fontName);
    ofsDec.flush();
    ofsDec.close();

    std::ofstream ofsDef{outputDir / (fontName + ".cpp")};
    exportFontDefinition(ofsDef, fontName, fontPath, imagePath, flipY);
    ofsDef.flush();
    ofsDef.close();
}

void fontproc::procFont(fs::path fontDir, fs::path outputDir, bool flipY)
{
    if(fs::is_directory(fontDir))
    {
        for (const auto& entry : fs::directory_iterator(fontDir))
        {
            if(entry.path().extension().string() == ".json")
            {
                exportFont(outputDir, entry.path(), flipY);
            }
        }
    }
    else if(fs::is_regular_file(fontDir))
    {
        exportFont(outputDir, fontDir, flipY);
    }
}