#pragma once

#include "ez_string.h"
#include "glcolor.h"
#include "glinputadapter.h"
#include "gltext.h"
#include "infinite_iterator.h"
#include <functional>
#include <memory>

namespace {
static inline std::optional<int> parseIntArg(const std::string &cmd) {
    const auto a = cmd.find("=");
    if (a != std::string::npos) {
        try {
            auto idx = std::stoi(cmd.substr(a + 1));
            return idx;
        } catch (...) {
        }
    }
    return std::nullopt;
}

static inline std::optional<std::string>
parseStringArg(const std::string &cmd) {
    const auto a = cmd.find("=");
    if (a != std::string::npos) {
        return cmd.substr(a + 1);
    }
    return std::nullopt;
}
} // namespace

namespace lix {

struct Console;

struct command {
    command(const char *shortname_) : shortname{shortname_} {}
    virtual bool perform(Console &console, const std::string &name) = 0;
    const char *shortname;
};

struct custom_command : public command {
    custom_command(
        const char *shortname_,
        const std::function<bool(Console &, const std::string &)> &callback_)
        : command(shortname_), callback{callback_} {}
    virtual bool perform(Console &console, const std::string &cmd) override {
        return callback(console, cmd);
    }
    std::function<bool(Console &, const std::string &)> callback;
};

template <typename Iterator> struct iterate_command : public command {
    iterate_command(const char *shortname_, Iterator &iter_)
        : command(shortname_), iter{iter_} {}
    virtual bool perform(Console &, const std::string &cmd) override {
        if (auto idx = parseIntArg(cmd)) {
            if (idx >= 0 && idx < iter.m_container.size()) {
                iter.set(static_cast<size_t>(*idx));
            } else {
                return false;
            }
        } else {
            ++iter;
        }
        return true;
    }
    Iterator &iter;
};

struct color_command : public command {
    color_command(const char *shortname_, lix::Color &color_)
        : command(shortname_), _color{color_} {}
    color_command(const char *shortname_,
                  std::function<lix::Color &()> color_fn_)
        : command(shortname_), _color{color_fn_()}, color_fn{color_fn_} {}
    virtual bool perform(Console &console, const std::string &name) override;

    Color &color() {
        if (color_fn) {
            return color_fn();
        }
        return _color;
    }

    lix::Color &_color;
    std::function<lix::Color &()> color_fn;
};

struct Console : public lix::InputAdapter {
    enum class ColorMode {
        NONE,
        RED,
        GREEN,
        BLUE,
        YELLOW,
        MAGENTA,
        CYAN,
        HUE,
        SATURATION,
        VALUE
    };

    enum class CommandResult {
        NOT_FOUND,
        PARSE_FAILURE,
        SUCCESS,
        SUCCESS_KEEP_OPEN
    };

    explicit Console(std::string &&label_, std::shared_ptr<lix::Text> textNode_)
        : label{std::move(label_)}, textNode{textNode_} {
        textNode->setVisible(false);
    }

    bool onMouseDown(KeySym key, KeyMod mod) override;
    bool onMouseUp(KeySym key, KeyMod mod) override;
    bool onMouseMove(float x, float y, float xrel, float yrel) override;
    bool onMouseWheel(float x, float y) override;
    bool onKeyDown(KeySym key, KeyMod mod) override;
    bool onKeyUp(KeySym key, KeyMod mod) override;

    CommandResult runCommand(const std::string &cmd);

    void updatePrefix();
    void updateText();
    void close();

    const std::string label;
    std::shared_ptr<lix::Text> textNode;
    std::vector<std::shared_ptr<command>> commands;
    bool focused{false};
    std::string prefix;
    std::string text;
    ColorMode colorMode{ColorMode::NONE};
    lix::Color restoreColor{lix::Color::magenta};
    lix::Color intermediateColor{lix::Color::magenta};
    float H, S, V;
    std::shared_ptr<command> executedCommand;
    std::string executedCmdLine;
};
} // namespace lix