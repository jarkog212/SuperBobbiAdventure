#pragma once
#include "H_Spike.h"
#include <box2d/box2d.h>

//constructor, uses base for hazzard
H_Spike::H_Spike(gef::Platform& platform, std::map<std::string, gef::Scene*>* models, gef::Renderer3D* renderer, b2World* phys_engine, Vector4 rotation):
	Hazzard(platform, GetMeshFromSceneAssets((*models)["Models/Spike.scn"]),HazzardTypes::SPIKE,renderer)
{
	setupPhysics(phys_engine, RigidBody_Type::Box, b2BodyType::b2_staticBody, 1, 0.65, 1);  //specific phys body
	Rotate(rotation);                                                                       //initial rotation, is not changed after this
}
