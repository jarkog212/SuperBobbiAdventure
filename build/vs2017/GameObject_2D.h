#pragma once
#include "GameObject.h"
#include <graphics/sprite.h> 


namespace gef {
	class SpriteRenderer;
}

//groupiong of sprite setup variables

struct SpriteSetup {
	float posX;
	float posY;
	float ordZ;
	float width;
	float height;
	const char* tex_address = "";   //texture address if not texture pointer
	gef::Texture* texture = NULL;   //priority
};

//a 2D sublass of GameObject

class GameObject_2D :
    public GameObject
{
protected:
	gef::Sprite sprite_;                              //The gef format of sprite
	gef::Texture* texture_;                           //texture to be used, pointer for potential reuse of textures
	gef::SpriteRenderer* sprite_renderer_ = NULL;     //sprite renderer is global
	bool texture_owner_ = false;                      //texture owner, flag to ensure that when deleting this object it wont delete the texture unless its the owner of it

	void applyTransform() override;                   //2D version of tranform_data to coordinates

public:
	GameObject_2D(gef::Platform& platform, SpriteSetup setup, gef::SpriteRenderer* sprite_renderer);    //no base constructor
	virtual ~GameObject_2D();
	void RenderObject() override;                                           //2D objects require specific set of instructions to be rendered, different form 3D
	
	//getters and setters
	gef::Sprite* getSprite() { return &sprite_; };                     
	gef::Texture* getTexture() { return texture_; };
	void setProtectTexture(bool setting) { texture_owner_ = setting; }
};

