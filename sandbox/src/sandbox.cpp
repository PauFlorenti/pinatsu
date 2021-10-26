#include "entry.h"

class Sandbox : public Application
{
public:
    Sandbox()
    {
    }
};

Application* CreateApplication()
{
    return new Sandbox();
}