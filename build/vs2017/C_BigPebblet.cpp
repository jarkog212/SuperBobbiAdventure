#pragma once
#include "C_BigPebblet.h"
#include "box2d/box2D.h"

C_BigPebblet::C_BigPebblet(gef::Platform& platform, std::map<std::string, gef::Scene*>* models, gef::Renderer3D* renderer, b2World* phys_engine) :
	Collectible(platform, models, renderer, phys_engine, 0.35,0.35,"Models/Pebblet3.scn")            //specifies the size of the hitbox and the model to load, not a good implementation 
{
	score_value_ = 250;          // sets the value                    
}
