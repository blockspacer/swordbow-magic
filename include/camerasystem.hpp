#ifndef CAMERASYSTEM_HPP
#define CAMERASYSTEM_HPP

//TODO: Use lowpoly3d

#include "isystem.hpp"

typedef unsigned int ID;
class IRenderer;
class CameraSystem : public ISystem {
private:
    ID cameraSource = 0;
    IRenderer* const renderer;

public:
    CameraSystem(IRenderer* const renderer);
    void add(ID id);
    void remove(ID id);
    void update();
    unsigned int count() const;
    const System::Identifier getIdentifier() const;
    bool activateId(ID id);
};


#endif //CAMERASYSTEM_HPP
