#pragma once

#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
#include <GLES3/gl32.h>
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include <SDL2/SDL.h>
#ifdef _WIN32
#undef main
#endif

#include <functional>

#include "glm/glm.hpp"

#include "glinputadapter.h"
#include "gltimer.h"
#include "gltypes.h"

namespace lix {
enum class DragState { START_DRAG, DRAGGING, END_DRAG };
using MouseDragCallback = std::function<void(KeySym, KeyMod, DragState)>;

class Application {
  public:
    Application(float windowX, float windowY, const char *title = "Untitled");

#ifndef __ANDROID__
    SDL_Window *window() const;
#endif
    void initialize();
    void run(bool forever = true);
    virtual void init() = 0;
    virtual void tick(float dt) = 0;
    virtual void draw() = 0;

    void pause() { _paused = true; }
    void resume() { _paused = false; }
    void step(Time::Raw ms) {
        tick(ms * 1e-3f);
        Time::increment(ms);
    }
    bool paused() { return _paused; }

    void setOnKeyDown(KeySym key,
                      const std::function<void(KeySym, KeyMod)> &keyCallback);
    void setOnKeyUp(KeySym key,
                    const std::function<void(KeySym, KeyMod)> &keyCallback);
    void setOnMouseDown(KeySym key,
                        const std::function<void(KeySym, KeyMod)> &keyCallback);
    void setOnMouseUp(KeySym key,
                      const std::function<void(KeySym, KeyMod)> &keyCallback);
    void setOnMouseDrag(KeySym key, const MouseDragCallback &keyCallback);

    void setInputAdapter(lix::InputAdapter *inputAdapter);
    lix::InputAdapter *inputAdapter() const;

    const glm::vec2 &windowSize() const;
    const glm::vec2 &mousePoistion() const;

    void setDrawableSize(int x, int y);
    const glm::ivec2 &getDrawableSize() const;

    glm::vec2
    normalizedMousePosition() const; // n.x: -1.0f - 1.0f == left - right, n.y:
                                     // -1.0f - 1.0f == bottom - top

    float fps() const;
    void setTickFrequency(lix::Time::Raw hz);

    void quit();

    void setMousePosition(int x, int y) {
        _mousePosition.x = (float)x;
        _mousePosition.y = (float)y;
    }

  protected:
    glm::ivec2 drawableSize;

  private:
    static void loop();

    void handleMouseMotion(float x, float y, float xrel, float yrel);
    void handleMouseButtonDown(lix::KeySym button, lix::KeyMod mod, float x,
                               float y);
    void handleMouseButtonUp(lix::KeySym button, lix::KeyMod mod, float x,
                             float y);
    void handleMouseWheel(float x, float y);
    void handleKeyDown(lix::KeySym key, lix::KeyMod mod);
    void handleKeyUp(lix::KeySym key, lix::KeyMod mod);

    enum KeyAction { KEY_DOWN, KEY_UP, MOUSE_DOWN, MOUSE_UP, LAST };
    const char *_title;
#ifndef __ANDROID__
    SDL_Window *_window{nullptr};
    SDL_GLContext _glContext{nullptr};
#endif
    float _fps{0.0f};
    lix::Time::Raw _tickFrequency{100};
    lix::Time::Raw _deltaTime{1000 / _tickFrequency};
    float _deltaTimeSeconds{_deltaTime * 1e-3f};
    bool _forever{false};
    glm::vec2 _windowSize;
    glm::vec2 _mousePosition;
    lix::InputAdapter *_inputAdapter{nullptr};
    std::unordered_map<KeySym, std::function<void(KeySym, KeyMod)>>
        _callbacks[KeyAction::LAST];
    inline void handleCallback(KeyAction keyAction, KeySym key,
                               KeyMod mod) const;
    struct DragHandler {
        MouseDragCallback callback;
        DragState dragState{DragState::END_DRAG};
    };
    std::unordered_map<KeySym, DragHandler> _mouseDragCallbacks;
    DragHandler *getDragHandler(KeySym);
    lix::KeySym _stickyButton;
    lix::KeyMod _stickyMod;
    bool _paused{false};
};
} // namespace lix