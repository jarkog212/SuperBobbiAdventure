#pragma once
#include "GameObject_3D.h"
#include <map>
#include <string>


//enum for all the enemy types
enum class HazzardTypes : int{
    SPIKE,
    BLOCKY,
    SPINNER    //unused
};

//base class for all the hazzards, both enemies and immovable hazzards

class Hazzard :
    public GameObject_3D
{
protected:
    HazzardTypes hazzardType_;
public:
    Hazzard(gef::Platform& platform, gef::Mesh* mesh, HazzardTypes enemyType, gef::Renderer3D* renderer) : GameObject_3D(platform, mesh, renderer) { type_ = ObjectType::HAZZARD; hazzardType_ = enemyType; };
    HazzardTypes getHazzardType() { return hazzardType_; };
};

