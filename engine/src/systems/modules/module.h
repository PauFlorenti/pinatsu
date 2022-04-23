#pragma once

class IModule {
public:
    IModule(std::string name) : name(name){};

    const std::string& getName() { return name; }
    bool isActive() { return active; }

protected:
    virtual bool start() { return true; }
    virtual void stop() {};
    virtual void update(f32 dt) {};
    virtual void render() {};
    virtual void renderUI() {};
    virtual void renderDebug() {};
    virtual void renderUIDebug() {};
    virtual void renderInMenu() {};

private:

    bool doStart()
    {
        PASSERT(!active)
        if(active) return false;

        const bool ok = start();
        if(ok)
            active = true;
        return ok;
    }

    void doStop()
    {
        PASSERT(active)
        if(!active) return;

        stop();
        active = false;
    }

    friend class CHandleManager;

    std::string name;
    bool active = false;
};