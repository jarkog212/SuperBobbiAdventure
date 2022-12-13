#pragma once
#include "Collectible.h"

//Class representing the Big collectible, worth 250 points, has its own model but is otherwise same to the base collectible class

class C_BigPebblet :
    public Collectible
{
public:
    C_BigPebblet(gef::Platform& platform, std::map<std::string, gef::Scene*>* models, gef::Renderer3D* renderer, b2World* phys_engine);
};

