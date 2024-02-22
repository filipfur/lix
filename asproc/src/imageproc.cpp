#include "imageproc.h"

#define STB_IMAGE_IMPLEMENTATION
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "stb_image.h"
#pragma clang diagnostic pop

unsigned char* imageproc::loadImage(const fs::path& imageFile, bool flipY, int& width, int& height, int& channels)
{
    stbi_set_flip_vertically_on_load(flipY);
    return stbi_load(imageFile.c_str(), &width, &height, &channels, 0);
}

void imageproc::freeImage(unsigned char* data)
{
    stbi_image_free(data);
}

const std::string channelSuffix(const std::string& name, int channels)
{
    std::string channelStr{""};
    switch(channels)
    {
        case 1:
            channelStr = "_red";
            break;
        case 3:
            channelStr = "_rgb";
            break;
        case 4:
            channelStr = "_rgba";
            break;
        default:
            break;
    }
    return name + channelStr;
}

void exportImageDeclaration(const fs::path& outputDir,
    const std::string& imageName,
    const std::string& suffixedName)
{
    const auto headerFile = (imageName + ".h");
    std::ofstream ofs{outputDir / headerFile};
    ofs << "#pragma once\n\nnamespace assets {\n    namespace images {\n";
    ofs << "        struct " << suffixedName << " {";
    ofs << "\n            static unsigned char data[];";
    ofs << "\n            static unsigned int width;";
    ofs << "\n            static unsigned int height;";
    ofs << "\n            static unsigned int channels;";
    ofs << "\n        };\n";
    ofs << "\n    }\n}";
    common::log("Write: " + headerFile);
    ofs.flush();
    ofs.close();
}

void exportImageDefinition(const fs::path& outputDir,
    const fs::path& imagePath,
    const std::string& imageName,
    std::string& suffixedName,
    bool flipOnLoad)
{
    int width;
    int height;
    int channels;
    unsigned char* data = imageproc::loadImage(imagePath, flipOnLoad, width, height, channels);
    /*common::log("Read: fileName=" + fileName + ", " + std::to_string(width)
        + "x" + std::to_string(height) + ", channels=" + std::to_string(channels));*/

    suffixedName = channelSuffix(imageName, channels);
    const auto imageCpp = imageName + ".cpp";

    if(!data)
    {
        std::cerr << "Failed to read image file:" << imagePath << std::endl;
        return;
    }

    std::ofstream ofs{outputDir / imageCpp};
    ofs << "#include \"" << imageName << ".h\"\n"
        << "unsigned char assets::images::" << suffixedName << "::data[] = {";
    
    int numBytes = width * height * channels;

    int pixelWidth = width * channels;
    std::ios_base::fmtflags f( ofs.flags() );
    for(int b{0}; b < numBytes - 1; ++b)
    {
        if(b % pixelWidth == 0) { ofs << "\n  "; }
        ofs << "0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(data[b]) << ',';
    }
    ofs << "0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(data[numBytes - 1]);
    ofs.flags( f );
    ofs << "\n};\n";

    ofs << "unsigned int assets::images::" << suffixedName << "::width{" << width << "};\n";
    ofs << "unsigned int assets::images::" << suffixedName << "::height{" << height << "};\n";
    ofs << "unsigned int assets::images::" << suffixedName << "::channels{" << channels << "};\n"; 

    common::log("Write: " + imageCpp);
    ofs.flush();
    ofs.close();
    imageproc::freeImage(data);
}

void exportImage(const fs::path& outputDir, const fs::path& imagePath, bool flipOnLoad)
{
    const std::string fileName = imagePath.filename().string();
    const std::string imageName = common::variableName(fileName.substr(0, fileName.rfind('.')));
    std::string suffixedName;
    exportImageDefinition(outputDir, imagePath, imageName, suffixedName, flipOnLoad);
    exportImageDeclaration(outputDir, imageName, suffixedName);
}

void imageproc::procImage(fs::path imageDir, fs::path outputDir, bool flipOnLoad)
{
    if(fs::is_directory(imageDir))
    {
        for (const auto& entry : fs::directory_iterator(imageDir))
        {
            exportImage(outputDir, entry.path(), flipOnLoad);
        }
    }
    else if(fs::is_regular_file(imageDir))
    {
        exportImage(outputDir, imageDir, flipOnLoad);
    }
}