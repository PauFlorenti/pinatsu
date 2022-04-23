#pragma once
#include "defines.h"

struct TEntityParseContext;

class CHandleManager;

template<class TObj>
class CObjectManager;

template<typename TObj>
CObjectManager<TObj>* getObjectManager();

class CHandle {
public:

    static const u32 nBitsType  = 7;
    static const u32 nBitsIndex = 14;
    static const u32 nBitsAge   = 32 - nBitsIndex - nBitsType;
    static const u32 maxTypes   = 1 << nBitsType;

    // Empty constructor. All zeros is an invalid handle.
    CHandle() : type(0), index(0), age(0) {};

    // Constructor with the three values.
    CHandle(u32 new_type, u32 new_index, u32 new_age)
        : type(new_type), index(new_index), age(new_age) {};

    template<class TObj>
    CHandle(TObj* objAddress) {
        auto hm = getObjectManager<std::remove_const<TObj>::type>();
        *this = hm->getHandleFromAddress(objAddress);
    }

    // Read-only getters.
    u32 getType()               const { return type; }
    u32 getIndex()              const { return index; }
    u32 getAge()                const { return age; }
    const char* getTypeName()   const;

    bool isValid() const;

    // Operators
    bool operator==(CHandle h) const {
        return type == h.type
        && index == h.index
        && age == h.age;
    }

    bool operator!=(CHandle h) const {
        return !(*this == h);
    }

    bool operator<(CHandle h) const {
        return type == h.type
        && index == h.index
        && age < h.age;
    }

    bool operator>(CHandle h) const {
        return type == h.type
        && index == h.index
        && age > h.age;
    }

    bool operator<=(CHandle h) const {
        return operator==(h) || operator<(h);
    }

    bool operator>=(CHandle h) const {
        return operator==(h) || operator>(h);
    }

    // Automatic cast
    template <class TObj>
    operator TObj* () const {
        // std::remove_const<T>::type returns TObj without const
        auto hm = getObjectManager<std::remove_const<TObj>::type>();
        return hm->getAddressFromHandle(*this);
    }

    // Create and destroy.
    template<class T>
    CHandle create() {
        auto h = getObjectManager<T>();
        *this = h->createHandle();
        return *this;
    }

    void destroy();

    void    setOwner(CHandle newOwner);
    CHandle getOwner();

    void debugInMenu();
    void renderDebug();
    void load(const json& j, TEntityParseContext& ctx);
    void onEntityCreated();

private:
    // Save n bits per each member. CHandle should only take 32 bits.
    u32 type : nBitsType;
    u32 index : nBitsIndex;
    u32 age : nBitsAge;
};