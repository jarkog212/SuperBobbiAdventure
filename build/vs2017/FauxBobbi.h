#pragma once
#pragma once
#include "PlayerBobbi.h"
class FauxBobbi :
    public PlayerBobbi
{
    //class for the fake player, inherits from player

    void handleControllerInput(float dt) override {};          //player input is specific, hecne override
    void collisionResponse(GameObject* collider) override {};  //collision response for the player
    void update(float dletaT) override;                        //false player update

public:
    //change the mesh to const char for file asset instead of a rpimitive
    FauxBobbi(gef::Platform& platform_, gef::Renderer3D* renderer, b2World* phys_engine);
    void reachedGoal() { ping_goal_ = true;  }    //sets up the goal conditions
    void setHits(int hits) { hits_ = hits; }
    void setState(int state) { state_ = state; }
};

