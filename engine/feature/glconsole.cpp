#include "glconsole.h"

#include "gltime.h"
#include <optional>
#include <string>

static lix::Console::ColorMode defaultColorMode{lix::Console::ColorMode::RED};

bool lix::color_command::perform(Console &console, const std::string &cmd) {
    if (auto hexcolor = parseStringArg(cmd)) {
        color() = lix::Color{*hexcolor};
    } else if (auto icolor = parseIntArg(cmd)) {
        // idk
    } else {
        console.colorMode = defaultColorMode;
    }
    return true;
}

bool lix::Console::onMouseDown(KeySym key, KeyMod mod) {
    (void)key;
    (void)mod;

    if (colorMode != ColorMode::NONE) {
        lix::Color &color =
            std::dynamic_pointer_cast<color_command>(executedCommand)->color();
        intermediateColor = color;
    }

    return false;
}

bool lix::Console::onMouseUp(KeySym key, KeyMod mod) {
    (void)key;
    (void)mod;
    return false;
}

float tweakFloat(float &f, float delta) {
    f = std::max(0.0f, std::min(1.0f, f + delta));
    return f;
}

bool lix::Console::onMouseMove(float x, float y, float xrel, float yrel) {
    (void)x;
    (void)y;
    (void)xrel;
    (void)yrel;

    float factor = (-yrel + xrel) * 0.002f;

    if (colorMode != ColorMode::NONE) {
        lix::Color &color =
            std::dynamic_pointer_cast<color_command>(executedCommand)->color();
        switch (colorMode) {
        case ColorMode::RED:
            text = std::to_string(
                (int)glm::round(tweakFloat(color.vec4().r, factor) * 255));
            break;
        case ColorMode::GREEN:
            text = std::to_string(
                (int)glm::round(tweakFloat(color.vec4().g, factor) * 255));
            break;
        case ColorMode::BLUE:
            text = std::to_string(
                (int)glm::round(tweakFloat(color.vec4().b, factor) * 255));
            break;
        case ColorMode::YELLOW:
            text = std::to_string(
                (int)glm::round(std::max(tweakFloat(color.vec4().r, factor),
                                         tweakFloat(color.vec4().g, factor)) *
                                255));
            break;
        case ColorMode::MAGENTA:
            text = std::to_string(
                (int)glm::round(std::max(tweakFloat(color.vec4().r, factor),
                                         tweakFloat(color.vec4().b, factor)) *
                                255));
            break;
        case ColorMode::CYAN:
            text = std::to_string(
                (int)glm::round(std::max(tweakFloat(color.vec4().g, factor),
                                         tweakFloat(color.vec4().b, factor)) *
                                255));
            break;
        case ColorMode::HUE:
            color = intermediateColor;
            H = std::max(-glm::pi<float>(),
                         std::min(glm::pi<float>(), H + factor));
            color.hueShift(H);
            text = std::to_string(H);
            break;
        case ColorMode::SATURATION:
            color = intermediateColor;
            S = std::max(0.0f, std::min(1.0f, S + factor));
            color.saturate(S);
            text = std::to_string(S);
            break;
        case ColorMode::VALUE:
            color = intermediateColor;
            V = std::max(0.0f, std::min(1.0f, V + factor));
            color.valueScale(V);
            text = std::to_string(V);
            break;
        default:
            break;
        }
        text += " (" + color.hexString() + ")";
        updateText();
        return true;
    }

    return false;
}

bool lix::Console::onMouseWheel(float x, float y) {
    (void)x;
    (void)y;
    return false;
}

void lix::Console::updatePrefix() {
    char token{'x'};
    switch (colorMode) {
    case ColorMode::RED:
        token = 'r';
        break;
    case ColorMode::GREEN:
        token = 'g';
        break;
    case ColorMode::BLUE:
        token = 'b';
        break;
    case ColorMode::YELLOW:
        token = 'y';
        break;
    case ColorMode::MAGENTA:
        token = 'm';
        break;
    case ColorMode::CYAN:
        token = 'c';
        break;
    case ColorMode::HUE:
        token = 'h';
        break;
    case ColorMode::SATURATION:
        token = 's';
        break;
    case ColorMode::VALUE:
        token = 'v';
        break;
    default:
        break;
    }
    prefix = executedCommand->shortname;
    prefix += '(';
    prefix += token;
    prefix += ')';
}

void lix::Console::updateText() { textNode->setText(label + prefix + text); }

void lix::Console::close() {
    prefix = "";
    text = "";
    focused = false;
    textNode->setVisible(false);
}

bool lix::Console::onKeyDown(KeySym key, KeyMod mod) {
    if (!focused) {
        if (key == SDLK_PERIOD) {
            if (mod & KMOD_SHIFT) {
                focused = true;
                textNode->setVisible(true);
                updateText();
                return true;
            } else if (executedCommand) {
                static float lastPeriod = 0.0f;
                float s = lix::Time::seconds();
                if (s - lastPeriod < 0.3f) {
                    executedCommand->perform(*this, executedCmdLine);
                    lastPeriod = 0.0f;
                    return true;
                }
                lastPeriod = s;
            }
        }
        return false;
    }

    if (colorMode != ColorMode::NONE) {
        switch (key) {
        case SDLK_RETURN:
            colorMode = ColorMode::NONE;
            close();
            break;
        case SDLK_ESCAPE:
            colorMode = ColorMode::NONE;
            std::dynamic_pointer_cast<color_command>(executedCommand)
                ->color()
                .vec4() = restoreColor.vec4();
            close();
            break;
        case SDLK_r:
            colorMode = (mod & KMOD_SHIFT) ? ColorMode::CYAN : ColorMode::RED;
            text = std::to_string((int)glm::round(
                std::dynamic_pointer_cast<color_command>(executedCommand)
                    ->color()
                    .vec4()
                    .r *
                255));
            updatePrefix();
            updateText();
            break;
        case SDLK_g:
            colorMode =
                (mod & KMOD_SHIFT) ? ColorMode::MAGENTA : ColorMode::GREEN;
            text = std::to_string((int)glm::round(
                std::dynamic_pointer_cast<color_command>(executedCommand)
                    ->color()
                    .vec4()
                    .g *
                255));
            updatePrefix();
            updateText();
            break;
        case SDLK_b:
            colorMode =
                (mod & KMOD_SHIFT) ? ColorMode::YELLOW : ColorMode::BLUE;
            text = std::to_string((int)glm::round(
                std::dynamic_pointer_cast<color_command>(executedCommand)
                    ->color()
                    .vec4()
                    .b *
                255));
            updatePrefix();
            updateText();
            break;
        case SDLK_h:
            colorMode = ColorMode::HUE;
            updatePrefix();
            updateText();
            break;
        case SDLK_s:
            colorMode = ColorMode::SATURATION;
            updatePrefix();
            updateText();
            break;
        case SDLK_v:
            colorMode = ColorMode::VALUE;
            updatePrefix();
            updateText();
            break;
        }
        return true;
    }

    int charcode = key - SDLK_a;
    int max = SDLK_z - SDLK_a;
    int numcode = key - SDLK_0;
    if (charcode >= 0 && charcode <= max) {
        text += char((mod & KMOD_SHIFT ? 'A' : 'a') + charcode);
    } else if (numcode >= 0 && numcode <= 9) {
        if (numcode == 0 && mod & KMOD_SHIFT) {
            text += '=';
        } else {
            text += char('0' + numcode);
        }
    } else if (key == SDLK_BACKSPACE) {
        if (text.length() < 2) {
            text = "";
        } else {
            text.erase(text.end() - 1);
        }
    } else if (key == SDLK_SPACE) {
        text += ' ';
    } else if (key == SDLK_PERIOD) {
        text += (mod & KMOD_SHIFT) ? ':' : '.';
    } else if (key == SDLK_COMMA) {
        text += (mod & KMOD_SHIFT) ? ';' : ',';
    } else if (key == SDLK_ESCAPE) {
        close();
    } else if (key == SDLK_RETURN) {
        auto res = runCommand(text);
        switch (res) {
        case CommandResult::SUCCESS_KEEP_OPEN:
            break;
        default:
            close();
            break;
        }
    }
    updateText();
    return true;
}

bool lix::Console::onKeyUp(KeySym, KeyMod) { return false; }

lix::Console::CommandResult lix::Console::runCommand(const std::string &cmd) {
    for (const auto &c : commands) {
        if (starts_with(cmd.c_str(), c->shortname)) {
            executedCommand = c;
            executedCmdLine = cmd;
            if (c->perform(*this, cmd)) {
                if (colorMode != ColorMode::NONE) {
                    updatePrefix();
                    restoreColor =
                        std::dynamic_pointer_cast<color_command>(c)->color();
                    intermediateColor = restoreColor;
                    auto hsv = restoreColor.hsv();
                    H = hsv[0];
                    S = hsv[1];
                    V = hsv[2];
                    text = std::to_string(
                        (int)glm::round(restoreColor.vec4().r * 255));
                    return CommandResult::SUCCESS_KEEP_OPEN;
                }
                return CommandResult::SUCCESS;
            } else {
                return CommandResult::PARSE_FAILURE;
            }
        }
    }
    return CommandResult::NOT_FOUND;
}