#include "entry.h"

#include "core/input.h"
#include "core/logger.h"

class Sandbox : public Application
{
public:
    Sandbox()
    {
    }

    bool update(f32 deltaTime);
    bool render();
};

Application* CreateApplication()
{
    return new Sandbox();
}

bool Sandbox::update(f32 deltaTime)
{

    if(isKeyDown(KEY_B))
    {
        PDEBUG("Key B is down!");
    }

    return true;
}