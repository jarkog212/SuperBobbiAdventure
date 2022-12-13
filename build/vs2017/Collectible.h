#pragma once
#include "GameObject_3D.h"
#include <map>
#include <string>

//enum of collectible types, used by level in spawning from the txt file

enum class CollectibleTypes{
    PEBBLET = 0,
    BIG_PEBBLET,
    HEART,                     //unused
    BANDAID,                   //unused
    GOAL};

//main base class for collectibles, base classs behaves like a simple pebblet, overrideable by children

class Collectible :
    public GameObject_3D 
{
protected:
    int score_value_ = 100;                                          //score to be added when collided with
    bool collected_ = false;                                         //collection flag, can only turn true, no reset available. Need to delete and initialise a new one

    virtual void collisionResponse(GameObject* collider) override;   //collision function, based on the function of the collectible
    virtual void update(float deltaT) override;                      //base update only rotates the model, virtual
public:
    void RenderObject() override;                                    //special rendering, hen lights need to be disabled to have an "emissive" look, hence the override. collectibles need to distinguisheable
    Collectible(gef::Platform& platform, std::map<std::string, gef::Scene*>* models, gef::Renderer3D* renderer, b2World* phys_engine, float phys_width = 0.25f, float phys_height = 0.25f, std::string model_key = "Models/Pebblet.scn");
};

