#pragma once

#include "core/application.h"

int main()
{
    Application* app = CreateApplication();

    if(!app->init())
    {
        return -1;
    }

    if(!app->run())
    {
        return -2;
    }

    delete app;

    return 0;
}