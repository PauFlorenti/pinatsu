#pragma once
#include "defines.h"

class CHandleManager;

class CHandle {
public:

    static const u32 nBitsType = 7;
    static const u32 nBitsIndex = 14;
    static const u32 nBitsAge = 32 - nBitsIndex - nBitsType;
    static const u32 maxTypes = 1 << nBitsType;

    CHandle() : type(0), index(0), age(0) {}

    CHandle(u32 new_type, u32 new_index, u32 new_age)
        : type(new_type), index(new_index), age(new_age) {}

    u32 getType()               const { return type; }
    u32 getIndex()              const { return index; }
    u32 getAge()                const { return age; }
    const char* getTypeName()   const;

    bool isValid() const;

    // Create and destroy.
    template<class T>
    CHandle create() {
        auto h = getObjectManaget<T>();
        *this = h->createHandle();
        return *this;
    }

    void destroy();

private:

    u32 type;
    u32 index;
    u32 age;
};