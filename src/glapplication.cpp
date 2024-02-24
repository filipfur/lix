#include "glapplication.h"

lix::Application* context{nullptr};

void loop()
{
    static GLuint previousTicks{0};
    GLuint ticks = SDL_GetTicks();
    GLuint deltaTicks = ticks - previousTicks;
    previousTicks = ticks;

    float dt = deltaTicks * 1e-3f;
    context->setTime(context->time() + dt);
    context->tick(dt);
    context->draw();

    SDL_GL_SwapWindow(context->window());
}

lix::Application::Application(int windowX, int windowY, const char* title)
    : _windowSize{static_cast<float>(windowX), static_cast<float>(windowY)}, _mousePosition{}
{
    SDL_Init(SDL_INIT_EVERYTHING);
#if __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    emscripten_trace_configure_for_google_wtf();
#else
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    _window = SDL_CreateWindow(title, 0, 0, windowX, windowY, SDL_WINDOW_OPENGL);
    _glContext = SDL_GL_CreateContext(_window);

    glewExperimental = GL_TRUE;
    glewInit();
}

SDL_Window* lix::Application::window() const { return _window; }

float lix::Application::time() const { return _time; }

void lix::Application::setTime(float time) { _time = time; }

void lix::Application::setOnKeyDown(KeySym key, const KeyCallback& keyCallback)
{
    _callbacks[lix::Application::KeyAction::KEY_DOWN][key] = keyCallback;
}

void lix::Application::setOnKeyUp(KeySym key, const KeyCallback& keyCallback)
{
    _callbacks[lix::Application::KeyAction::KEY_UP][key] = keyCallback;
}

void lix::Application::setOnMouseDown(KeySym key, const KeyCallback& keyCallback)
{
    _callbacks[lix::Application::KeyAction::MOUSE_DOWN][key] = keyCallback;
}

void lix::Application::setOnMouseUp(KeySym key, const KeyCallback& keyCallback)
{
    _callbacks[lix::Application::KeyAction::MOUSE_UP][key] = keyCallback;
}

void lix::Application::setOnMouseDrag(KeySym key, const MouseDragCallback& mouseDragCallback)
{
    _mouseDragCallbacks[key] = {mouseDragCallback, DragState::END_DRAG};
}

void lix::Application::setInputAdapter(lix::InputAdapter* inputAdapter)
{
    _inputAdapter = inputAdapter;
}

lix::InputAdapter* lix::Application::inputAdapter() const
{
    return _inputAdapter;
}

const glm::vec2& lix::Application::windowSize() const
{
    return _windowSize;
}

const glm::vec2& lix::Application::mousePoistion() const
{
    return _mousePosition;
}

glm::vec2 lix::Application::normalizedMousePosition() const
{
    return _mousePosition / _windowSize;
}

void lix::Application::handleCallback(lix::Application::KeyAction keyAction,
    lix::KeySym key, lix::KeyMod mod) const
{
    auto& map = _callbacks[keyAction];
    auto it = map.find(key);
    if(it != map.end()) {
        it->second(key, mod);
    }
}

lix::Application::DragHandler* lix::Application::getDragHandler(KeySym key)
{
    auto it = _mouseDragCallbacks.find(key);
    if(it != _mouseDragCallbacks.end())
    {
        return &it->second;
    }
    return nullptr;
}

void lix::Application::run(bool forever)
{
    context = this;
    this->init();
#if __EMSCRIPTEN__
    emscripten_set_main_loop(loop, -1, 1);
#else
    SDL_Event event;
    bool doLoop{true};
    _forever = forever;
    while (doLoop)
    {
        if (SDL_PollEvent(&event))
        {
            static KeySym key{0};
            static KeySym button{0};
            static KeyMod mod{0};
            lix::Application::DragHandler* dh{nullptr};
            switch(event.type)
            {
                case SDL_QUIT:
                    _forever = false;
                    break;
                case SDL_MOUSEMOTION:
                    _mousePosition.x = static_cast<float>(event.motion.x);
                    _mousePosition.y = static_cast<float>(event.motion.y);
                    dh = getDragHandler(button);
                    if(dh && dh->dragState != lix::DragState::END_DRAG)
                    {
                        dh->callback(button, mod, lix::DragState::DRAGGING);
                        dh->dragState = lix::DragState::DRAGGING;
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    _mousePosition.x = static_cast<float>(event.button.x);
                    _mousePosition.y = static_cast<float>(event.button.y);
                    button = event.button.button;
                    mod = SDL_GetModState();
                    if(_inputAdapter) { _inputAdapter->onMouseDown(key, mod); }
                    handleCallback(MOUSE_DOWN, key, mod);
                    dh = getDragHandler(button);
                    if(dh && dh->dragState == lix::DragState::END_DRAG)
                    {
                        dh->callback(button, mod, lix::DragState::START_DRAG);
                        dh->dragState = lix::DragState::START_DRAG;
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    _mousePosition.x = static_cast<float>(event.button.x);
                    _mousePosition.y = static_cast<float>(event.button.y);
                    button = event.button.button;
                    mod = SDL_GetModState();
                    if(_inputAdapter) { _inputAdapter->onMouseUp(key, mod); }
                    handleCallback(MOUSE_UP, key, mod);
                    dh = getDragHandler(button);
                    if(dh)
                    {
                        dh->callback(button, mod, lix::DragState::END_DRAG);
                        dh->dragState = lix::DragState::END_DRAG;
                    }
                    break;
                case SDL_KEYDOWN:
                    key = event.key.keysym.sym;
                    mod = event.key.keysym.mod;
                    if(_inputAdapter) { _inputAdapter->onKeyDown(key, mod); }
                    handleCallback(KEY_DOWN, key, mod);
                    break;
                case SDL_KEYUP:
                    key = event.key.keysym.sym;
                    mod = event.key.keysym.mod;
                    if(_inputAdapter) { _inputAdapter->onKeyUp(key, mod); }
                    handleCallback(KEY_UP, key, mod);
                    break;
            }
        }
        loop();
        doLoop = _forever;
    }
#endif

    SDL_GL_DeleteContext(_glContext);
    SDL_Quit();
}

void lix::Application::quit()
{
    _forever = false;
}