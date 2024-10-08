#include "glapplication.h"

#include <cassert>

lix::Application *context{nullptr};

void lix::Application::loop() {
#ifndef __ANDROID__
    static SDL_Event event;
    if (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            context->_forever = false;
            break;
        case SDL_MOUSEMOTION:
            context->handleMouseMotion(static_cast<float>(event.motion.x),
                                       static_cast<float>(event.motion.y),
                                       static_cast<float>(event.motion.xrel),
                                       static_cast<float>(event.motion.yrel));
            break;
        case SDL_MOUSEBUTTONDOWN:
            context->handleMouseButtonDown(
                event.button.button,
                static_cast<lix::KeyMod>(SDL_GetModState()),
                static_cast<float>(event.button.x),
                static_cast<float>(event.button.y));
            break;
        case SDL_MOUSEBUTTONUP:
            context->handleMouseButtonUp(
                event.button.button,
                static_cast<lix::KeyMod>(SDL_GetModState()),
                static_cast<float>(event.button.x),
                static_cast<float>(event.button.y));
            break;
        case SDL_MOUSEWHEEL:
            context->handleMouseWheel(static_cast<float>(event.wheel.x),
                                      static_cast<float>(event.wheel.y));
            break;
        case SDL_KEYDOWN:
            context->handleKeyDown(event.key.keysym.sym, event.key.keysym.mod);
            break;
        case SDL_KEYUP:
            context->handleKeyUp(event.key.keysym.sym, event.key.keysym.mod);
            break;
        }
    }

    static uint32_t previousTicks{0};
    uint32_t ticks = SDL_GetTicks();
    static uint32_t deltaTicks{0};
    deltaTicks += (ticks - previousTicks);
    previousTicks = ticks;

    static Timer timer10Hz{lix::Time::fromHz<10>()};
    static size_t frames{0};

    while (deltaTicks >= context->_deltaTime) {

        if (!context->_paused) {
            context->tick(context->_deltaTimeSeconds);
        }

        context->draw();
        ++frames;

        if (timer10Hz.elapsed()) {
            context->_fps = frames * 10.0f;
            frames = 0;
            timer10Hz.reset();
        }

        SDL_GL_SwapWindow(context->window());

        deltaTicks -= context->_deltaTime;

        if (!context->_paused) {
            lix::Time::increment(context->_deltaTime);
        }
    }

    if (context->_fps > 60) {
        SDL_Delay(1);
    }
#endif
}

lix::Application::Application(float windowX, float windowY, const char *title)
    : drawableSize{static_cast<int>(windowX), static_cast<int>(windowY)},
      _title{title},
      _windowSize{static_cast<float>(windowX), static_cast<float>(windowY)},
      _mousePosition{} {}

#ifndef __ANDROID__
SDL_Window *lix::Application::window() const { return _window; }
#endif

void lix::Application::setOnKeyDown(
    KeySym key, const std::function<void(KeySym, KeyMod)> &keyCallback) {
    _callbacks[lix::Application::KeyAction::KEY_DOWN][key] = keyCallback;
}

void lix::Application::setOnKeyUp(
    KeySym key, const std::function<void(KeySym, KeyMod)> &keyCallback) {
    _callbacks[lix::Application::KeyAction::KEY_UP][key] = keyCallback;
}

void lix::Application::setOnMouseDown(
    KeySym key, const std::function<void(KeySym, KeyMod)> &keyCallback) {
    _callbacks[lix::Application::KeyAction::MOUSE_DOWN][key] = keyCallback;
}

void lix::Application::setOnMouseUp(
    KeySym key, const std::function<void(KeySym, KeyMod)> &keyCallback) {
    _callbacks[lix::Application::KeyAction::MOUSE_UP][key] = keyCallback;
}

void lix::Application::setOnMouseDrag(
    KeySym key, const MouseDragCallback &mouseDragCallback) {
    _mouseDragCallbacks[key] = {mouseDragCallback, DragState::END_DRAG};
}

void lix::Application::setInputAdapter(lix::InputAdapter *inputAdapter) {
    _inputAdapter = inputAdapter;
}

lix::InputAdapter *lix::Application::inputAdapter() const {
    return _inputAdapter;
}

const glm::vec2 &lix::Application::windowSize() const { return _windowSize; }

void lix::Application::setDrawableSize(int x, int y) {
    drawableSize.x = x;
    drawableSize.y = y;
}

const glm::ivec2 &lix::Application::getDrawableSize() const {
    return drawableSize;
}

const glm::vec2 &lix::Application::mousePoistion() const {
    return _mousePosition;
}

glm::vec2 lix::Application::normalizedMousePosition() const {
    return _mousePosition / _windowSize;
}

float lix::Application::fps() const { return _fps; }

void lix::Application::setTickFrequency(lix::Time::Raw hz) {
    _tickFrequency = hz;
    assert(1000 % hz == 0); // not nice numbers
    _deltaTime = 1000 / hz;
    _deltaTimeSeconds = _deltaTime * 1e-3f;
}

void lix::Application::handleCallback(lix::Application::KeyAction keyAction,
                                      lix::KeySym key, lix::KeyMod mod) const {
    auto &map = _callbacks[keyAction];
    auto it = map.find(key);
    if (it != map.end()) {
        it->second(key, mod);
    }
}

lix::Application::DragHandler *lix::Application::getDragHandler(KeySym key) {
    auto it = _mouseDragCallbacks.find(key);
    if (it != _mouseDragCallbacks.end()) {
        return &it->second;
    }
    return nullptr;
}

// TO BE CONTINUED
// https://github.com/emscripten-core/emscripten/blob/main/test/test_html5_core.c
// https://gist.github.com/nus/564e9e57e4c107faa1a45b8332c265b9

void lix::Application::initialize() {
    static bool initialized = false;
    if (initialized) {
        return;
    }
    initialized = true;
#ifndef __ANDROID__
    SDL_Init(SDL_INIT_EVERYTHING);
#ifdef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
    // SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif

    _window = SDL_CreateWindow(_title, 0, 0, static_cast<int>(_windowSize.x),
                               static_cast<int>(_windowSize.y),
                               SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
    _glContext = SDL_GL_CreateContext(_window);
    assert(SDL_GL_MakeCurrent(_window, _glContext) == 0);

    SDL_GL_GetDrawableSize(_window, &drawableSize.x, &drawableSize.y);

#ifndef __EMSCRIPTEN__
    glewExperimental = GL_TRUE;
    glewInit();
#endif

#endif // __ANDROID__

    context = this;
    this->init();
}

void lix::Application::run(bool forever) {
    initialize();
#ifndef __ANDROID__
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, -1, 1);
#else
    bool doLoop{true};
    _forever = forever;
    while (doLoop) {
        loop();
        doLoop = _forever;
    }
#endif
    SDL_GL_DeleteContext(_glContext);
    SDL_Quit();
#endif // __ANDROID__
}

void lix::Application::handleMouseMotion(float x, float y, float xrel,
                                         float yrel) {
    _mousePosition.x = x;
    _mousePosition.y = y;
    if (_inputAdapter) {
        _inputAdapter->onMouseMove(x, y, xrel, yrel);
    }
    auto dh = getDragHandler(_stickyButton);
    if (dh && dh->dragState != lix::DragState::END_DRAG) {
        dh->callback(_stickyButton, _stickyMod, lix::DragState::DRAGGING);
        dh->dragState = lix::DragState::DRAGGING;
    }
}

void lix::Application::handleMouseButtonDown(lix::KeySym button,
                                             lix::KeyMod mod, float x,
                                             float y) {
    _mousePosition.x = x;
    _mousePosition.y = y;
    if (_inputAdapter) {
        _inputAdapter->onMouseDown(button, mod);
    }
    handleCallback(MOUSE_DOWN, button, mod);
    auto dh = getDragHandler(button);
    if (dh && dh->dragState == lix::DragState::END_DRAG) {
        dh->callback(button, mod, lix::DragState::START_DRAG);
        dh->dragState = lix::DragState::START_DRAG;
    }
    _stickyButton = button;
    _stickyMod = mod;
}

void lix::Application::handleMouseButtonUp(lix::KeySym button, lix::KeyMod mod,
                                           float x, float y) {
    _mousePosition.x = x;
    _mousePosition.y = y;
    if (_inputAdapter) {
        _inputAdapter->onMouseUp(button, mod);
    }
    handleCallback(MOUSE_UP, button, mod);
    auto dh = getDragHandler(button);
    if (dh) {
        dh->callback(button, mod, lix::DragState::END_DRAG);
        dh->dragState = lix::DragState::END_DRAG;
    }
}

void lix::Application::handleMouseWheel(float x, float y) {
    if (_inputAdapter) {
        _inputAdapter->onMouseWheel(x, y);
    }
}

void lix::Application::handleKeyDown(lix::KeySym key, lix::KeyMod mod) {
#ifndef __ANDROID__
    if (_paused) {
        switch (key) {
        case SDLK_c:
            _paused = false;
            break;
        case SDLK_n:
            step(_deltaTime);
            break;
        }
    };
#endif
    if (_inputAdapter) {
        _inputAdapter->onKeyDown(key, mod);
    }
    handleCallback(KEY_DOWN, key, mod);
}

void lix::Application::handleKeyUp(lix::KeySym key, lix::KeyMod mod) {
    if (_inputAdapter) {
        _inputAdapter->onKeyUp(key, mod);
    }
    handleCallback(KEY_UP, key, mod);
}

void lix::Application::quit() { _forever = false; }