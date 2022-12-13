#pragma once
#include "GameObject_3D.h"

class PlayerBobbi;

// simple object for a fan, used to visualise the input, 

class Fan :
    public GameObject_3D
{
    PlayerBobbi* player_;                           //needed for origin position, essentially hierarchical modeling

    void handleControllerInput(float dT) override;  //determines the direction away from player where to show the object, relative
    Vector4 calculatePositionFromPlayer();          //uses the player as origin and applies its own transform based on that
public:
    Fan(gef::Platform &platform, gef::InputManager* input, gef::Renderer3D* renderer, PlayerBobbi* player) ;   // no base constructor
    void RenderObject() override;                   //needs a modification to disable lights, fan should be clearly visible, behaves more like a hud object in that sense
};

