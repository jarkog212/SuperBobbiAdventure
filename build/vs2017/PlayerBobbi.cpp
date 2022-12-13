#pragma once
#include "PlayerBobbi.h"
#include <box2d/box2d.h>
#include <graphics/scene.h>
#include <graphics/mesh.h>
#include <system/string_id.h>

#include <chrono>
#include <thread>
#include "Globals.h"

//define for the y position that is considered a kill zone
#define DEATH_Y -3

//external link to global variables
extern Globals globals;
extern Network network;

//handle controller input, override
void PlayerBobbi::handleControllerInput(float dt)
{
	if (controller_input_->IsR2Held()) {                                                                                 //if R2 is held...

		if (elapsed_fan_time == 0) {                                                                                     //...play sound, since it cannot loop need to restart every time after it finishes 
			setupEmitter(NULL, globals.SFX_map["Fan"], false);
		}
		elapsed_fan_time += dt;                                                                                          
		if (elapsed_fan_time >= 0.115) {                                                                                 
			elapsed_fan_time = 0;
		}

		b2Vec2 wind = b2Vec2(0, 0);                                                                                      //...create a widn vector based on the left stick
		wind.x = controller_input_->controls()->left_stick_x_axis() * -1 * globals.G_invert_X_axis * force_multiplier;
		wind.y = controller_input_->controls()->left_stick_y_axis() * globals.G_invert_Y_axis * force_multiplier;

		phys_body_->ApplyForceToCenter(wind, true);                                                                      //...apply the force to the player

		if (getResponsiveCollider()) {                                                                                       //...if player is responsive...
			const_cast<gef::Mesh*>(mesh_)->GetPrimitive(0)->set_material(scene_assets_list->materials_map[FanMaterialID]);   //... ...set the texture to be the happy face
			state_ = 1;                                                                                                      //saves the happy state
		}
	}
	else if (getResponsiveCollider()) {                                                                                 //if R2 is NOT held, but player is responsive...
		const_cast<gef::Mesh*>(mesh_)->GetPrimitive(0)->set_material(scene_assets_list->materials_map[MainMaterialID]); //...set the texture to be the default face
		elapsed_fan_time = 0;                                                                                           //disable fan SFX
		audio_3D_->getAudioManager()->StopPlayingSampleVoice(0);              
		state_ = 0;                                                                                                     //saves the default state
	}
	else {                                                           //otherwise...
		elapsed_fan_time = 0;                                        //just disable the fan sfx
		audio_3D_->getAudioManager()->StopPlayingSampleVoice(0);
	}

	if (controller_input_->IsStartHeld()) {            //if start is pressed send a ping to level to switch to pause
		ping_pause_ = true; 
	}
}

//collision response
void PlayerBobbi::collisionResponse(GameObject* collider)
{
	switch (collider->getType()) {                                                                                      //based on the type of the collider
	case ObjectType::HAZZARD:                                                                                           //if player hits the enemy...
		hits_--;                                                                                                            //...decrease health (hits)
		if (hits_ > 0) {                                                                                                    //...if hits are stil above 0...
			audio_3D_->getAudioManager()->PlaySample(globals.SFX_map["Player_hit"]);                                        //... ...play hit SFX
			timer_ = INVULNERABILITY_TIME;                                                                                  //... ...make player invulnereable for a little while
			is_responsive_collider_ = false;                                                                                //... ...disable collision response for a while
			const_cast<gef::Mesh*>(mesh_)->GetPrimitive(0)->set_material(scene_assets_list->materials_map[HurtMaterialID]); //... ...dset the texture of the face to hurt
			state_ = 2;                                                                                                  //saves the hurt state
		}
		
		break;
	case ObjectType::ENVIRO:                                                                                  //if player hits environment...
		if (abs(phys_body_->GetLinearVelocity().x) > 0.3f && abs(phys_body_->GetLinearVelocity().y) > 0.3f)   //...ensure that the speed was high enough for...
			audio_3D_->getAudioManager()->PlaySample(globals.SFX_map["Player_boing"]);                        //...the SFX of boing to play
		break;
	}
}

//update override
void PlayerBobbi::update(float deltaT)
{
	
	if ((hits_ == 0 || transform_data_.position_.y() < DEATH_Y) &&   //if hits is at 0 or the player fell too low... 
		network.role == NetworkRole::None)                         //...abd if not a networked game...
	{                                                               
		death();                                                   //...kill the player
	} 
	if (wait(timer_, deltaT)) {                                    //wait for some time
		return;
	}
	is_responsive_collider_ = true;                                //reenable responsive collider
}

//death function
void PlayerBobbi::death()
{
	audio_3D_->getAudioManager()->StopMusic();                            //stops the music
	audio_3D_->getAudioManager()->PlaySample(globals.SFX_map["Death"]);   //plays death SFX
	std::this_thread::sleep_for(std::chrono::milliseconds(5500));         //wait for death music to finish
	audio_3D_->getAudioManager()->PlayMusic();                            //play the level theme again
	--lives_;                                                             //decrement lives
	setPosition(start_position_);                                         //restart the player at the beginning
	phys_body_->SetLinearVelocity(b2Vec2(0, 0));                          //resets the speed/velocity
	hits_ = MAX_HITS;                                                     //resets the hits
	timer_ = 0.2;                                                         //sets the timer to ensure no collision with hazzard happens right after respawn
	is_responsive_collider_ = false;                                      //disables responsiveness
	temp_elapsed_time = 0;                                                //resets the elapsed time
}

//constructor, loads the player mesh
PlayerBobbi::PlayerBobbi(gef::Platform& platform, gef::Renderer3D* renderer, gef::InputManager* input, b2World* phys_engine, int hits):
	GameObject_3D(platform,"Models/Bobbi.scn",renderer)
{
	if(input)
		setupControllerInput(input);                                                           //setsup the controller input, has input manager
	hits_ = hits;
	toggleResponsiveCollider();                                                                //enables responsiveness
	setObjectType(ObjectType::PLAYER);                                                         //set the type to be the player
	setupPhysics(phys_engine, RigidBody_Type::Sphere, b2BodyType::b2_dynamicBody, 1, 1, 1);    //setsup the circuar phys body
	getBody()->GetFixtureList()->SetRestitution(0.8f);                                         //sets up the restitution not 1
	getBody()->SetGravityScale(0.4f);                                                          //floaty ball gravity

	for (auto it : scene_assets_list->material_data_map) {                                     //save the texture IDS for all the face variants
		if (it.second->diffuse_texture == "Bobbi_Fan_T.png") {
			FanMaterialID = it.second->name_id;
		}
		else if (it.second->diffuse_texture == "Bobbi_Hurt_T.png") {
			HurtMaterialID = it.second->name_id;
		}
		else if (it.second->diffuse_texture == "Bobbi_T.png") {
			MainMaterialID = it.second->name_id;
		}
	}
}

//sets the position and save it as starting point
void PlayerBobbi::placePlayer(Vector4 position)
{
	setPosition(position);
	start_position_ = position;
}

//returns a ping if applicable, plays SFX
bool PlayerBobbi::getPingPause()
{
	if (ping_pause_) {
		ping_pause_ = false;
		audio_3D_->getAudioManager()->PlaySample(globals.SFX_map["Pause"]);
		return true;
	}
	return false;
}
