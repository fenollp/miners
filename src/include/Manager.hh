#pragma once

#include <Arguments.hh>
#include <Scene.hh>
#include <Events.hh>

class Manager {
public:
    Manager(Arguments* args) : fullscreen_(false), args_(args) {}
    virtual ~Manager() {}

    virtual void loadScene(Scene* scene) {
        scene_ = scene;
    }

    virtual void init() = 0;
    virtual void run() = 0;

    virtual Events* getEvents(int id=0) = 0;
    virtual Mouse* getMouse(int id=0) = 0;

    virtual void toggleFullscreen() = 0;

    Arguments* args() { return args_; }

protected:
    bool fullscreen_;
    Arguments* args_;
    Scene* scene_;
};

using ManagerFactoryFunc = std::function<Manager*(Arguments*)>;

extern std::map<std::string, ManagerFactoryFunc> managers;
