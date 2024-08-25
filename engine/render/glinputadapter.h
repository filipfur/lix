#pragma once

#include "gltypes.h"

namespace lix {
class InputAdapter {
  public:
    virtual bool onMouseDown(KeySym key, KeyMod mod) = 0;
    virtual bool onMouseUp(KeySym key, KeyMod mod) = 0;
    virtual bool onMouseMove(float x, float y, float xrel, float yrel) = 0;
    virtual bool onMouseWheel(float x, float y) = 0;
    virtual bool onKeyDown(KeySym key, KeyMod mod) = 0;
    virtual bool onKeyUp(KeySym key, KeyMod mod) = 0;
};
} // namespace lix