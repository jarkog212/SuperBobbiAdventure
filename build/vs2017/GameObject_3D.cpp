#pragma once
#include "GameObject_3D.h"
#include <graphics/renderer_3d.h>
#include <maths/math_utils.h>
#include <box2D/box2D.h>
#include <graphics/mesh.h>
#include <system/platform.h>
#include "graphics/scene.h"

//updates the sound, overriden 
void GameObject_3D::updateSound()
{
	if (listener_) {                             //simplifies the listener transform by using already built matrix of the mesh instead of building a new one
		listener_->SetTransform(transform_);
		
	}
	if (!emitters_.size() != 0) {                                                          //same as base
		for (auto it = emitters_.begin(); it != emitters_.end(); it++) {
			(*it)->set_position(transform_data_.position_);
			if (!(*it)->looping()) {
				audio_3D_->getAudioManager()->StopPlayingSampleVoice((*it)->voice_id());
				emitters_.erase(it);
			}
		}
	}
}

//apply transform, build the matrix from transfomr data to be used by the mesh, literally before render hence phys is reflected already
void GameObject_3D::applyTransform()
{
	transform_.SetIdentity();                                           //clear prev transform

	gef::Matrix44 temp;                         
	temp.SetIdentity();

	temp.Scale(transform_data_.scale_);                                 //create scale matrix
	transform_ = transform_ * temp;                                     //concatenate with transform (transform * scale)
	temp.SetIdentity();

	temp.RotationX(gef::DegToRad(transform_data_.rotation_.x()));       //create rotation X axis matrix
	transform_ = transform_ * temp;                                     //concatenate with transform (transform * rotation X)
	temp.SetIdentity();

	temp.RotationY(gef::DegToRad(transform_data_.rotation_.y()));       //create rotation Y axis matrix
	transform_ = transform_ * temp;                                     //concatenate with transform (transform * rotation Y)
	temp.SetIdentity();

	temp.RotationZ(gef::DegToRad(transform_data_.rotation_.z()));       //create rotation Z axis matrix
	transform_ = transform_ * temp;                                     //concatenate with transform (transform * rotation Z)
	temp.SetIdentity();

	temp.SetTranslation(transform_data_.position_);                     //create translate axis matrix    
	transform_ = transform_ * temp;                                     //concatenate with transform (transform * translate)
	temp.SetIdentity();

}

//constructor for 3D object, can be supplied with a mesh, not loading a new one then
GameObject_3D::GameObject_3D(gef::Platform& platform, const gef::Mesh* mesh, gef::Renderer3D* renderer, bool mesh_owner, int networkID ) :
	GameObject(platform)
{
	set_mesh(mesh);                 //sets the loaded mesh as the object mesh
	is_mesh_owner = mesh_owner;     //sets whether this has the ability to delete the mesh, usually not in this case, deprecated actually
	renderer_ = renderer;           //global renderer pointer
	networkID_ = networkID;         //network ID to identify objects
}

//conxtructor for 3D object, supplied with the address to the scene
GameObject_3D::GameObject_3D(gef::Platform& platform, const char* scene_asset_filename, gef::Renderer3D* renderer) :
	GameObject(platform)
{
	gef::Scene* scene_assets_;                    
	renderer_ = renderer;

	scene_assets_ = LoadSceneAssets(platform_, scene_asset_filename);             //loads the scene from a file
	if (scene_assets_)                                                            //id successful...
	{
		set_mesh(GetMeshFromSceneAssets(scene_assets_));                          //sets the first mesh as THE object mesh, in truth scene is the owner of the mesh, not the object
	} 
	else                                                                          //debug, deprecated
	{
		gef::DebugOut("Scene file %s failed to load\n", scene_asset_filename);
	}
}

//destructor combined with the base one
GameObject_3D::~GameObject_3D()
{
	if (mesh_ && is_mesh_owner) {      //if mesh exists and is mesh owner, delete it (depecated as scene is ALWAYS the owner of the mesh not object, this is just in case)
		delete mesh_;                  //the check also prevents deleting an already deleted object as only the owner can delete
	}
	if (scene_assets_list) {           //if there is a personal scene asset, delete it, object scene asset cannot be shared, takes care of the dangling pointers
		delete scene_assets_list;
		scene_assets_list = NULL;
		mesh_ = NULL;
	}
}

//render override, using 3D functions to render, conditioned with a flag
void GameObject_3D::RenderObject()
{
	if (is_visible) {
		applyTransform();
		renderer_->DrawMesh(*this);
	}
}

//-----------code provided by lecturer //
gef::Scene* GameObject_3D::LoadSceneAssets(gef::Platform& platform, const char* filename)
{
	gef::Scene* scene = new gef::Scene();

	if (scene->ReadSceneFromFile(platform, filename))
	{
		// if scene file loads successful
		// create material and mesh resources from the scene data
		scene->CreateMaterials(platform);
		scene->CreateMeshes(platform);
		scene_assets_list = scene;
	}
	else
	{
		delete scene;
		scene = NULL;
	}

	return scene;
}

gef::Mesh* GameObject_3D::GetMeshFromSceneAssets(gef::Scene* scene)
{
	gef::Mesh* mesh = NULL;

	// if the scene data contains at least one mesh
	// return the first mesh
	if (scene && scene->meshes.size() > 0) {
		mesh = scene->meshes.front();
	}

	return mesh;
}
