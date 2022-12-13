#pragma once
#include "C_Goal.h"
#include "PlayerBobbi.h"
#include "Globals.h"

//external link to the gloabal variables
extern Globals globals;

//goal collectible

void C_Goal::collisionResponse(GameObject* collider)
{
	if (collider->getType() == ObjectType::PLAYER) {                          //if player is the collider...
		PlayerBobbi* player = static_cast<PlayerBobbi*>(collider);            //...cast the pointer to the player, safe
		player->reachedGoal();                                                //...set the ping for the goal
		audio_3D_->getAudioManager()->PlaySample(globals.SFX_map["Win"]);     //...play the SFX
	}
}

C_Goal::C_Goal(gef::Platform& platform, std::map<std::string, gef::Scene*>* models, gef::Renderer3D* renderer, b2World* phys_engine) :
	Collectible(platform, models, renderer, phys_engine, 1, 1, "Models/Goal.scn")   // Load the goal model
{
	score_value_ = 0;          //goal has no score
}