#pragma once
#include "Hazzard.h"

// a static spike enemy, only changes a bit like the size of the hitbox, hence only constructor

class H_Spike :
    public Hazzard
{
public:
    H_Spike(gef::Platform& platform, std::map<std::string, gef::Scene*>* models, gef::Renderer3D* renderer, b2World* phys_engine, Vector4 rotation);
};

