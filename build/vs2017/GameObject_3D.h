#pragma once
#include "GameObject.h"
#include "stdlib.h"
#include <graphics/mesh_instance.h>
#include <system/debug_log.h>
#include <list>
#include <time.h>

using gef::MeshInstance;

typedef unsigned long       DWORD;

namespace gef {
    class Renderer3D;
    class Scene;
}

//3D version of game object, also inherits from mesh instance

class GameObject_3D :
    public MeshInstance, public GameObject
{
protected:
    void updateSound() override;                                                                                        //uses less resourcess as mesh instance already has amatrix to use, no additional calculation needed
    void applyTransform() override;                                                                                     //builds mesh instance's transform matrix from transform_data

    bool is_mesh_owner = false;                                                                                         //meshes can be reused,when deleting deletes the mesh only if it owns it

    gef::Renderer3D* renderer_;                                                                                         //renderer is global
    gef::Scene* scene_assets_list = NULL;                                                                               //its possible to have a scene in object, should be used sparingly as it makes reuse difficult
    int networkID_ = -1;                                                                                                     // object ID for networking
    DWORD lastNetworkUpdate_ = 0;
    b2Vec2 previousVelocity_ = b2Vec2(0, 0);

public:
    GameObject_3D(gef::Platform& platform, const gef::Mesh* mesh, gef::Renderer3D* renderer, bool mesh_owner = false, int networkID = -1);  //no base constructor
    GameObject_3D(gef::Platform& platform, const char* scene_asset_filenameconst, gef::Renderer3D* renderer);
    virtual ~GameObject_3D();

    virtual void RenderObject() override;                                                            //3D objects have specidfic sets of instructions for render which differ from 2D objects
    gef::Scene* LoadSceneAssets(gef::Platform& platform, const char* filename);       //loads scene assets file
    gef::Mesh* GetMeshFromSceneAssets(gef::Scene* scene);                             //gets the first mesh from the scene
    int GetNetworkID() { return networkID_; };
    void SetNetworkID(int ID) { networkID_ < 0 ? networkID_ = ID : 0; }
    void SetLastNetworkUpdate(DWORD timeStamp) { lastNetworkUpdate_ = timeStamp; };   //sets the last update timestamp from server
    DWORD GetLastNetworkUpdate() { return lastNetworkUpdate_; };                      //access ...
    b2Vec2 GetPreviousVelocity() { return previousVelocity_; }
    void SetPreviousVelocity(b2Vec2 prev) { previousVelocity_ = prev; }
};

