#pragma once
#include "Collectible.h"
#include "box2d/box2d.h"
#include <graphics/scene.h>
#include "PlayerBobbi.h"
#include <graphics/renderer_3d.h>
#include "Globals.h"

//external link to the gloabal variables
extern Globals globals;

//collision response
void Collectible::collisionResponse(GameObject* collider)
{
	if (collider->getType() == ObjectType::PLAYER) {                                 //if player...
		PlayerBobbi* player = static_cast<PlayerBobbi*>(collider);                   //...casts the collider pointer into the player type
		player->addScore(score_value_);                                              //add score from the variable
		collected_ = true;                                                           //sets the collected flag
		setVisibility(false);                                                        //disables the rendering
		audio_3D_->getAudioManager()->PlaySample(globals.SFX_map["Collectible"]);    //plays SFX
	}
}

//base update
void Collectible::update(float deltaT)
{
 	if (collected_) { phys_body_->SetEnabled(false); }    //if collected then disables the physics
	temp_elapsed_time += deltaT;                          //adds the time to the stopwatch for rotation
	rotateNoPhys(Vector4(0, 360 * deltaT, 0));            //adds more rotaion baased on the delta t
}

//render function
void Collectible::RenderObject()
{
	if (is_visible) {                                                                                 //if visible the does render 
		gef::Colour temp_ = renderer_->default_shader_data().ambient_light_colour();                  //saves the light data...
		renderer_->default_shader_data().set_ambient_light_colour(gef::Colour(0.7, 0.7, 0.7));        //... because it disables them for rendering
		applyTransform();
		renderer_->DrawMesh(*this);
		renderer_->default_shader_data().set_ambient_light_colour(temp_);                             //reaplies the light data
	}
	
}

//constructor
Collectible::Collectible(gef::Platform& platform, std::map<std::string, gef::Scene*>* models, gef::Renderer3D* renderer, b2World* phys_engine,float phys_width, float phys_height,std::string model_key) :
	GameObject_3D(platform,GetMeshFromSceneAssets((*models)[model_key]),renderer)
{               
	setObjectType(ObjectType::COLLECTIBLE);                                                                    //collectible type
	setupPhysics(phys_engine, RigidBody_Type::Box, b2BodyType::b2_dynamicBody, phys_width,phys_height,1);      //setsup the physics body
	toggleResponsiveCollider();                                                                                //enables collision repsonse
	phys_body_->SetGravityScale(0);                                                                            //0 gravity
	phys_body_->GetFixtureList()[0].SetSensor(true);                                                           //no physical response to collisino, purely detection
}
