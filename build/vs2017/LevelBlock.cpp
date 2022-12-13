#pragma once
#include "LevelBlock.h"
#include <box2d/box2d.h>
#include <graphics/scene.h>

//constructor, uses the level block type to load the model, clunky
LevelBlock::LevelBlock(gef::Platform& platform, LevelBlockTypes blockType, std::map<std::string,gef::Scene*>* models ,gef::Renderer3D* renderer, b2World* phys_engine, bool is_collider):
	GameObject_3D(platform,"",renderer)
{
	setObjectType(ObjectType::ENVIRO);                                           //all level block objects are of ENVIRO type
	b2Vec2 phys_dimension;
	switch (blockType) {                                                         //based on type, loads the model and specifies the phys body dimensions
	case LevelBlockTypes::M1x1:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/Mechanical_1x1.scn"]);
		phys_dimension = b2Vec2(1, 1);
		break;
	case LevelBlockTypes::M2x1:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/Mechanical_2x1.scn"]);
		phys_dimension = b2Vec2(2, 1);
		break;
	case LevelBlockTypes::M3x1:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/Mechanical_3x1.scn"]);
		phys_dimension = b2Vec2(3, 1);
		break;
	case LevelBlockTypes::M4x4:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/Mechanical_4x4.scn"]);
		phys_dimension = b2Vec2(2, 2);
		break;
	case LevelBlockTypes::G2x1:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/Ground_2x1.scn"]);
		phys_dimension = b2Vec2(2, 1);
		break;
	case LevelBlockTypes::G3x1:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/Ground_3x1.scn"]);
		phys_dimension = b2Vec2(3, 1);
		break;
	case LevelBlockTypes::G4x1:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/Ground_4x1.scn"]);
		phys_dimension = b2Vec2(4, 1);
		break;
	case LevelBlockTypes::G4x4:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/Ground_4x4.scn"]);
		phys_dimension = b2Vec2(2, 2);
		break;
	case LevelBlockTypes::MGR1:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/MegaGroup1.scn"]);
		phys_dimension = b2Vec2(8, 8);
		break;
	case LevelBlockTypes::GRP2:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/Group2.scn"]);
		phys_dimension = b2Vec2(4, 4);
		break;
	case LevelBlockTypes::C1x1:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/CityArena_1x1.scn"]);
		phys_dimension = b2Vec2(1, 1);
		break;
	case LevelBlockTypes::C1x2:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/CityArena_1x2.scn"]);
		phys_dimension = b2Vec2(2, 1);
		break;
	case LevelBlockTypes::C13A:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/CityArena_1x3.scn"]);
		phys_dimension = b2Vec2(3, 1);
		break;
	case LevelBlockTypes::C13B:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/CityArena_1x32.scn"]);
		phys_dimension = b2Vec2(3, 1);
		break;
	case LevelBlockTypes::C4x4:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/BlueMechanical_4x4.scn"]);
		phys_dimension = b2Vec2(2, 2);
		break;
	case LevelBlockTypes::GC01:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/GroupCity1.scn"]);
		phys_dimension = b2Vec2(3, 17);
		break;
	case LevelBlockTypes::GC02:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/GroupCity2.scn"]);
		phys_dimension = b2Vec2(4, 4);
		break;
	case LevelBlockTypes::GC03:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/GroupCity3.scn"]);
		phys_dimension = b2Vec2(4, 4);
		break;
	case LevelBlockTypes::MGC1:
		mesh_ = GetMeshFromSceneAssets((*models)["Models/MegaGroupCity1.scn"]);
		phys_dimension = b2Vec2(8, 8);
		break;
	}

	if (is_collider) {                  //based on the param sets up the phys body, specifications of which are determined in the code above
		setupPhysics(phys_engine, RigidBody_Type::Box, b2BodyType::b2_staticBody, phys_dimension.x, phys_dimension.y, 0);
	}
}
