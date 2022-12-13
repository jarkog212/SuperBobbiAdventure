#pragma once
#include "GameObject_3D.h"
#include <map>
#include <string>

// enum of the level blocks 

enum class LevelBlockTypes {
    M1x1,
    M2x1,
    M3x1,
    M4x4,
    G2x1,
    G3x1,
    G4x1,
    G4x4,
    MGR1,
    GRP2,
    C1x1,
    C1x2,
    C13A,
    C13B,
    C2x3,
    C4x4,
    MGC1,
    GC01,
    GC02,
    GC03
};

//class for the level block, only difference between types is the mesh and size of the hitbox, hence only contructor

class LevelBlock :
    public GameObject_3D
{
public:
    LevelBlock(gef::Platform& platform, LevelBlockTypes blockType, std::map<std::string, gef::Scene*>* models, gef::Renderer3D* renderer, b2World* phys_engine, bool is_collider);

};

