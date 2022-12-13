#pragma once
#include "Hazzard.h"
#include <box2d/box2d.h>

//class for the spiked cube enemy, child of Hazzard

class H_Blocky :
    public Hazzard
{
    void update(float deltaT) override;                           //specifies behaviour of the movement
    void collisionResponse(GameObject* collider) override ;       //checks for collisions with the player AND other leves assets, changes direction if it hits a non-player

    Vector4 directions_ = Vector4(0,0,0,0);                       //used as a simple multiplier for determining which compass direction blocky moves in
    GameObject* last_collider_ = NULL;                            //used to ensure that the body doesnt keep colliding with the same level asset
    float speed_ = 3.0f;                                          //speed multiplier
    float wait_time = 0.0f;                                       //a holder for time, used with wait()

public:
    H_Blocky(gef::Platform& platform, std::map<std::string, gef::Scene*>* models, gef::Renderer3D* renderer, b2World* phys_engine, Vector4 initial_direction);
};

