#pragma once
#include "Collectible.h"

//collectible item for the goal post

class C_Goal :
    public Collectible
{
protected:
    void collisionResponse(GameObject* collider) override;            //collision override to call the goal related functions
    void update(float deltaT) override {};                            //is not rotating like the base class is, hence empty update
public:
    C_Goal(gef::Platform& platform, std::map<std::string, gef::Scene*>* models, gef::Renderer3D* renderer, b2World* phys_engine);
};

