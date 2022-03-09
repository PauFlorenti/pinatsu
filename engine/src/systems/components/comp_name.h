#pragma once

#include "comp_base.h"

class TCompName : public TCompBase
{
    static const size_t maxSize = 64;
    char name[maxSize];

public:
    TCompName::TCompName();
    const char* getName() {return name;}
    void setName(const char* newName);
    void debugInMenu();
};