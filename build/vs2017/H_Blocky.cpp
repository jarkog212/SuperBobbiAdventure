#pragma once
#include "H_Blocky.h"
#include <graphics/scene.h>
#include "Globals.h"

//update override
void H_Blocky::update(float deltaT)
{
	phys_body_->SetAwake(true);                                                                //ensures that no shenanigance happens to the body, probably not needed
	
	if (wait(wait_time, deltaT)) {                                                             //wait for next movement if hit a wall, the pause after it hits the wall
		return;
	}
	phys_body_->SetLinearVelocity(b2Vec2(directions_.x() * speed_, directions_.y() * speed_)); //sets the movement in phys temrs, direction is the varibale, rest are constants
}

//external link to global variables
extern Globals globals;

//collision response override
void H_Blocky::collisionResponse(GameObject* collider)
{
	if (collider != last_collider_ && (collider->getType() == ObjectType::ENVIRO || collider->getType() == ObjectType::HAZZARD)) { //if hit a hazzard or wall and not what hit it before...
		setupEmitter(NULL, globals.SFX_map["Blocky_boing"], false,8.0f);                                                           //...play sound
		last_collider_ = collider;                                                                                                 //...saves the last collider
		wait_time = 1;                                                                                                             //...ads a wait time of 1 second
		directions_ *= -1;                                                                                                         //...switches the directions to be the opposite
		phys_body_->SetLinearVelocity(b2Vec2(0, 0));                                                                               //...stops it from moving for now, in phys terms
	}
	if (collider->getType() == ObjectType::PLAYER) {                                           //if hits the player...
		b2Vec2 force_out(1,1);
		if (collider->getTransform().position_.x() < transform_data_.position_.x()) {          //...determines where the player came from, generally...
			force_out.x *= -1;                                                                 //...bounces the player off in that direction
		}
		if (collider->getTransform().position_.y() < transform_data_.position_.y()) {          //...
			force_out.y *= -1;
		}

		phys_body_->SetAwake(false);                                                           //starts shenanigance, to stop it from jiggling after collision, probably...
		collider->getBody()->ApplyLinearImpulseToCenter(force_out, true);                      //...not needed as it is a sensor
	}
}

//constructor, uses the base for hazzard
H_Blocky::H_Blocky(gef::Platform& platform, std::map<std::string, gef::Scene*>* models, gef::Renderer3D* renderer, b2World* phys_engine, Vector4 initial_direction):
	Hazzard(platform, GetMeshFromSceneAssets((*models)["Models/Blocky.scn"]),HazzardTypes::BLOCKY,renderer)
{
	setupPhysics(phys_engine, RigidBody_Type::Box, b2BodyType::b2_dynamicBody, 0.95f, 0.95f, 1);       //specific phys body
	toggleResponsiveCollider();                                                                        //enables collision response
	directions_ = initial_direction;                                                                   //initialises the direction
	phys_body_->SetFixedRotation(true);                                                                //disables the phys body to rotate
	phys_body_->SetGravityScale(0);                                                                    //disables the gravity for this object
	phys_body_->GetFixtureList()->SetSensor(true);                                                     //sets this as a sensor, phys engine only checks for collision, not for phys collision
	phys_body_->SetLinearVelocity(b2Vec2(directions_.x() * speed_ / 2, directions_.y() * speed_ / 2)); //default movement defined in terms of physics
}
