#pragma once

#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/trace.h>
#endif

#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <functional>

#include "glm.hpp"

#include "gltypes.h"
#include "glinputadapter.h"

namespace lix
{
    //bool onKeyPressed(int key);
    using KeyCallback = std::function<void(KeySym, KeyMod)>;
    enum class DragState{START_DRAG, DRAGGING, END_DRAG};
    using MouseDragCallback = std::function<void(KeySym, KeyMod, DragState)>;

    class Application
    {
    public:
        Application(int windowX, int windowY, const char* title="Untitled");

        SDL_Window* window() const;
        float time() const;
        void setTime(float time);
        void run(bool forever=true);
        virtual void init() = 0;
        virtual void tick(float dt) = 0;
        virtual void draw() = 0;
    
        void setOnKeyDown(KeySym key, const KeyCallback& keyCallback);
        void setOnKeyUp(KeySym key, const KeyCallback& keyCallback);
        void setOnMouseDown(KeySym key, const KeyCallback& keyCallback);
        void setOnMouseUp(KeySym key, const KeyCallback& keyCallback);
        void setOnMouseDrag(KeySym key, const MouseDragCallback& keyCallback);

        void setInputAdapter(lix::InputAdapter* inputAdapter);
        lix::InputAdapter* inputAdapter() const;

        const glm::vec2& windowSize() const;
        const glm::vec2& mousePoistion() const;
        glm::vec2 normalizedMousePosition() const; // n.x: -1.0f - 1.0f == left - right, n.y: -1.0f - 1.0f == bottom - top 

        void quit();
    private:
        enum KeyAction
        {
            KEY_DOWN,
            KEY_UP,
            MOUSE_DOWN,
            MOUSE_UP,
            LAST
        };

        SDL_Window* _window{nullptr};
        float _time{0.0f};
        bool _forever{false};
        SDL_GLContext _glContext{nullptr};
        glm::vec2 _windowSize;
        glm::vec2 _mousePosition;
        lix::InputAdapter* _inputAdapter{nullptr};
        std::unordered_map<KeySym, KeyCallback> _callbacks[KeyAction::LAST];
        inline void handleCallback(KeyAction keyAction, KeySym key, KeyMod mod) const;
        struct DragHandler{
            MouseDragCallback callback;
            DragState dragState{DragState::END_DRAG};
        };
        std::unordered_map<KeySym, DragHandler> _mouseDragCallbacks;
        DragHandler* getDragHandler(KeySym);
    };
}