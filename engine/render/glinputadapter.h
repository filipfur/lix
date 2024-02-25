#pragma once

#include "gltypes.h"

namespace lix
{
class InputAdapter
{
public:
    virtual void onMouseDown(KeySym key, KeyMod mod) = 0;
    virtual void onMouseUp(KeySym key, KeyMod mod) = 0;
    virtual void onKeyDown(KeySym key, KeyMod mod) = 0;
    virtual void onKeyUp(KeySym key, KeyMod mod) = 0;
};
}