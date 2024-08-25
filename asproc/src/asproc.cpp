#define LOG(stream) std::cout << stream << std::endl;

#include "fontproc.h"
#include "imageproc.h"
#include "objectproc.h"
#include "plyproc.h"
#include "shaderproc.h"
#include <cstring>

void usage() {
    std::cout << "usage: asproc [--version-override <version>] [-s shader_dir "
                 "output_dir]"
              << std::endl;
}

enum class Mode { NONE, SHADERS, IMAGES, OBJECTS, FONTS, PLY };

int main(int argc, const char *argv[]) {
    std::string versionOverride{""};
    fs::path inputDir;
    fs::path outputDir;
    Mode mode{Mode::NONE};
    bool flipOnLoad{false};
    bool convertToSrgb{false};
    if (argc < 2) {
        usage();
        return 0;
    }
    for (int i{1}; i < argc; ++i) {
        if (strcmp(argv[i], "-s") == 0) {
            if (i + 2 >= argc) {
                std::cerr << "Failed to parse arguments for option -s"
                          << std::endl;
                usage();
                return 1;
            }
            mode = Mode::SHADERS;
            inputDir = argv[i + 1];
            outputDir = argv[i + 2];
            i += 2;
        } else if (strcmp(argv[i], "-i") == 0) {
            if (i + 2 >= argc) {
                std::cerr << "Failed to parse arguments for option -i"
                          << std::endl;
                usage();
                return 1;
            }
            mode = Mode::IMAGES;
            inputDir = argv[i + 1];
            outputDir = argv[i + 2];
            i += 2;
        } else if (strcmp(argv[i], "--convert-to-srgb") == 0) {
            convertToSrgb = true;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 2 >= argc) {
                std::cerr << "Failed to parse arguments for option -o"
                          << std::endl;
                usage();
                return 1;
            }
            mode = Mode::OBJECTS;
            inputDir = argv[i + 1];
            outputDir = argv[i + 2];
            i += 2;
        } else if (strcmp(argv[i], "-p") == 0) {
            if (i + 2 >= argc) {
                std::cerr << "Failed to parse arguments for option -p"
                          << std::endl;
                usage();
                return 1;
            }
            mode = Mode::PLY;
            inputDir = argv[i + 1];
            outputDir = argv[i + 2];
            i += 2;
        } else if (strcmp(argv[i], "--version-override") == 0) {
            if (i + 1 >= argc) {
                std::cerr
                    << "Failed to parse arguments for option --version-override"
                    << std::endl;
                usage();
                return 1;
            }
            versionOverride = argv[i + 1];
            i += 1;
        } else if (strcmp(argv[i], "--flip-y") == 0) {
            flipOnLoad = true;
        } else if (strcmp(argv[i], "-f") == 0) {
            if (i + 2 >= argc) {
                std::cerr << "Failed to parse arguments for option -f"
                          << std::endl;
                usage();
                return 1;
            }
            mode = Mode::FONTS;
            inputDir = argv[i + 1];
            outputDir = argv[i + 2];
            i += 2;
        } else if (strcmp(argv[i], "-h") == 0) {
            usage();
            return 0;
        } else {
            std::cerr << "Unknown argument: " << argv[i] << std::endl;
            usage();
            return 1;
        }
    }

    if (!fs::is_directory(inputDir) && !fs::is_regular_file(inputDir)) {
        std::cerr << "No such file or directory: " << inputDir << std::endl;
        return 0;
    }
    if (!fs::is_directory(outputDir)) {
        fs::create_directories(outputDir);
    }

    switch (mode) {
    case Mode::NONE:
        break;
    case Mode::SHADERS:
        shaderproc::procShaders(inputDir, outputDir, versionOverride);
        break;
    case Mode::IMAGES:
        imageproc::procImage(inputDir, outputDir, flipOnLoad, convertToSrgb);
        break;
    case Mode::OBJECTS:
        objectproc::procObject(inputDir, outputDir, convertToSrgb);
        break;
    case Mode::PLY:
        plyproc::procPLY(inputDir, outputDir);
        break;
    case Mode::FONTS:
        fontproc::procFont(inputDir, outputDir, flipOnLoad);
        break;
    default:
        break;
    }

    return 0;
}