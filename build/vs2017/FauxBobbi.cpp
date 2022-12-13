#include "FauxBobbi.h"
#include <box2d/box2d.h>
#include <graphics/scene.h>
#include <graphics/mesh.h>
#include <system/string_id.h>

void FauxBobbi::update(float dletaT)
{
	switch (state_) {
		case 0:
			const_cast<gef::Mesh*>(mesh_)->GetPrimitive(0)->set_material(scene_assets_list->materials_map[MainMaterialID]);   //... ...set the texture to be the happy face
			break;
		case 1:
			const_cast<gef::Mesh*>(mesh_)->GetPrimitive(0)->set_material(scene_assets_list->materials_map[FanMaterialID]);   //... ...set the texture to be the happy face
			break;
		case 2:
			const_cast<gef::Mesh*>(mesh_)->GetPrimitive(0)->set_material(scene_assets_list->materials_map[HurtMaterialID]);   //... ...set the texture to be the happy face
			break;
	}
}

FauxBobbi::FauxBobbi(gef::Platform& platform_, gef::Renderer3D* renderer, b2World* phys_engine):
PlayerBobbi(platform_ , renderer, NULL, phys_engine, MAX_HITS_MULTI)
{                                                              
	setObjectType(ObjectType::FAUX_PLAYER);                                                         //set the type to be the false; player
}
