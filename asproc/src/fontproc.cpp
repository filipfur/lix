#include "fontproc.h"

#include "imageproc.h"
#include "json.h"
#include "ttf.h"

void exportFontDeclaration(std::ofstream &ofs, const std::string &fontName) {
    ofs << "#pragma once\n"
        << "#include \"gltftypes.h\"\n"
        << "#include \"ttf.h\"\n"
        << "namespace assets {\n"
        << "    namespace fonts {\n"
        << "        namespace " << fontName << " {\n"
        << "            extern const ttf::Font font;\n";
    ofs << "        }\n    }\n}\n";
    common::log("Write: " + fontName + ".h");
}

std::string charToString(char c) {
    std::string str{"'"};
    if (c == '\'' || c == '\\') {
        str += '\\';
    }
    str += c;
    str += "'";
    return str;
}

void exportJsonFont(std::ofstream &ofs, const fs::path &fontPath,
                    const std::string &scope) {
    json::Json obj;
    std::ifstream ifs{fontPath};
    ifs >> obj;

    ofs << "static const ttf::Character characters[] = {\n";

    std::vector<ttf::Character> characters(256);
    for (const auto &chObj : obj["characters"].children()) {
        char index = chObj.first.at(0);
        const auto &c = chObj.second;
        ttf::Character &character = characters.at(index);
        character.token = index;
        character.x = (float)c["x"].toInt();
        character.y = (float)c["y"].toInt();
        character.width = (float)c["width"].toInt();
        character.height = (float)c["height"].toInt();
        character.originX = (float)c["originX"].toInt();
        character.originY = (float)c["originY"].toInt();
        character.advance = (float)c["advance"].toInt();
    }

    std::string delim{""};
    for (int i{32}; i <= 32 + 94; ++i) {
        auto &character = characters.at(i);
        char tok = character.token;
        const std::string token =
            charToString(tok < ' ' ? ' ' : (tok > '~' ? '~' : tok));
        ofs << delim << "    {" << token << ", " << character.x << ", "
            << character.y << ", " << character.width << ", "
            << character.height << ", " << character.originX << ", "
            << character.originY << ", " << character.advance << "}";
        delim = ",\n";
    }
    ofs << "\n};\n";

    ofs << "const ttf::Font " << scope << "::font{"
        << std::quoted(obj["name"].value()) << ",\n"
        << "    " << obj["size"].toInt() << ", " << obj["width"].toInt() << ", "
        << obj["height"].toInt()
        << ", characters, imageData, imageWidth, imageHeight, "
           "imageChannels};\n";
}

void exportFontDefinition(std::ofstream &ofs, const std::string &fontName,
                          const fs::path &fontPath, const fs::path &imagePath,
                          bool flipY) {
    int width;
    int height;
    int channels;
    const std::string scope = "assets::fonts::" + fontName;
    unsigned char *data =
        imageproc::loadImage(imagePath, flipY, width, height, channels);
    ofs << "#include \"" << fontName << ".h\"\n";

    ofs << "static const unsigned char imageData[] = {";
    int numBytes = width * height * channels;
    int pixelWidth = width * channels;
    std::ios_base::fmtflags f(ofs.flags());
    for (int b{0}; b < numBytes - 1; ++b) {
        if (b % pixelWidth == 0) {
            ofs << "\n  ";
        }
        ofs << "0x" << std::hex << std::setfill('0') << std::setw(2)
            << static_cast<int>(data[b]) << ',';
    }
    ofs << "0x" << std::hex << std::setfill('0') << std::setw(2)
        << static_cast<int>(data[numBytes - 1]);
    ofs.flags(f);
    ofs << "\n};\n";
    ofs << "static const unsigned int imageWidth{" << width << "};\n";
    ofs << "static const unsigned int imageHeight{" << height << "};\n";
    ofs << "static const unsigned int imageChannels{" << channels << "};\n";

    exportJsonFont(ofs, fontPath, scope);

    imageproc::freeImage(data);

    common::log("Write: " + fontName + ".cpp");
}

void exportFont(const fs::path &outputDir, const fs::path &fontPath,
                bool flipY) {
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

void fontproc::procFont(fs::path fontDir, fs::path outputDir, bool flipY) {
    if (fs::is_directory(fontDir)) {
        for (const auto &entry : fs::directory_iterator(fontDir)) {
            if (entry.path().extension().string() == ".json") {
                exportFont(outputDir, entry.path(), flipY);
            }
        }
    } else if (fs::is_regular_file(fontDir)) {
        exportFont(outputDir, fontDir, flipY);
    }
}