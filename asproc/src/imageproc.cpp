#include "imageproc.h"

#define STB_IMAGE_IMPLEMENTATION
#ifdef _WIN32
#include "stb_image.h"
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include "stb_image.h"
#pragma clang diagnostic pop
#endif

#include "glm/glm.hpp"

unsigned char *imageproc::loadImage(const fs::path &imageFile, bool flipY,
                                    int &width, int &height, int &channels) {
    stbi_set_flip_vertically_on_load(flipY);
    std::string fileStr = imageFile.string();
    return stbi_load(fileStr.c_str(), &width, &height, &channels, 0);
}

void imageproc::freeImage(unsigned char *data) { stbi_image_free(data); }

const std::string channelSuffix(const std::string &name, int channels) {
    std::string channelStr{""};
    switch (channels) {
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

void exportImageDeclaration(const fs::path &outputDir,
                            const std::string &imageName,
                            const std::string &suffixedName,
                            const imageproc::ImageData &imageData) {
    const auto headerFile = (imageName + ".h");
    std::ofstream ofs{outputDir / headerFile};
    ofs << "#pragma once\n\nnamespace assets {\n    namespace images {\n";
    ofs << "        struct " << suffixedName << " {";
    ofs << "\n            static unsigned char data[];";
    ofs << "\n            inline static constexpr unsigned int width{"
        << imageData.width << "};";
    ofs << "\n            inline static constexpr unsigned int height{"
        << imageData.height << "};";
    ofs << "\n            inline static constexpr unsigned int channels{"
        << imageData.channels << "};";
    ofs << "\n        };\n";
    ofs << "\n    }\n}";
    common::log("Write: " + headerFile);
    ofs.flush();
    ofs.close();
}

void exportImageDefinition(const fs::path &outputDir, const fs::path &imagePath,
                           const std::string &imageName,
                           std::string &suffixedName, bool flipOnLoad,
                           bool convertToSrgb,
                           imageproc::ImageData &imageData) {
    unsigned char *data =
        imageproc::loadImage(imagePath, flipOnLoad, imageData.width,
                             imageData.height, imageData.channels);
    /*common::log("Read: fileName=" + fileName + ", " + std::to_string(width)
        + "x" + std::to_string(height) + ", channels=" +
       std::to_string(channels));*/

    int byteSize{imageData.width * imageData.height * imageData.channels};

    if (convertToSrgb) {
        for (int i{0}; i < byteSize; ++i) {
            float f = static_cast<float>(data[i]) / 255.0f;
            if (f <= 0.04045f) {
                f = f / 12.92f;
            } else {
                f = glm::pow((f + 0.055f) / 1.055f, 2.4f);
            }
            data[i] = static_cast<unsigned char>(f * 255.0f);
        }
        std::cout << "Converted to SRGB: " << imageName << std::endl;
    }

    suffixedName = channelSuffix(imageName, imageData.channels);
    const auto imageCpp = imageName + ".cpp";

    if (!data) {
        std::cerr << "Failed to read image file:" << imagePath << std::endl;
        return;
    }

    std::ofstream ofs{outputDir / imageCpp};
    ofs << "#include \"" << imageName << ".h\"\n"
        << "unsigned char assets::images::" << suffixedName << "::data[] = {";

    int numBytes = imageData.width * imageData.height * imageData.channels;

    int pixelWidth = imageData.width * imageData.channels;
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

    /*ofs << "unsigned int assets::images::" << suffixedName << "::width{" <<
    imageData.width << "};\n"; ofs << "unsigned int assets::images::" <<
    suffixedName << "::height{" << imageData.height << "};\n"; ofs << "unsigned
    int assets::images::" << suffixedName << "::channels{" << imageData.channels
    << "};\n";*/

    common::log("Write: " + imageCpp);
    ofs.flush();
    ofs.close();
    imageproc::freeImage(data);
}

void exportImage(const fs::path &outputDir, const fs::path &imagePath,
                 bool flipOnLoad, bool convertToSrgb) {
    const std::string fileName = imagePath.filename().string();
    const std::string imageName = common::variableName(
        fileName /*fileName.substr(0, fileName.rfind('.'))*/);
    std::string suffixedName;
    imageproc::ImageData imageData;
    exportImageDefinition(outputDir, imagePath, imageName, suffixedName,
                          flipOnLoad, convertToSrgb, imageData);
    exportImageDeclaration(outputDir, imageName, suffixedName, imageData);
}

void imageproc::procImage(fs::path imageDir, fs::path outputDir,
                          bool flipOnLoad, bool convertToSrgb) {
    if (fs::is_directory(imageDir)) {
        for (const auto &entry : fs::directory_iterator(imageDir)) {
            exportImage(outputDir, entry.path(), flipOnLoad, convertToSrgb);
        }
    } else if (fs::is_regular_file(imageDir)) {
        exportImage(outputDir, imageDir, flipOnLoad, convertToSrgb);
    }
}