#pragma once
#include "GameObject_2D.h"
#include <assets/png_loader.h>
#include <graphics/image_data.h>
#include <graphics/texture.h>
#include <graphics/sprite_renderer.h>
#include <box2D/box2D.h>

//apply transform specific for 2D
void GameObject_2D::applyTransform()
{
	sprite_.set_position(transform_data_.position_);                     //match sprite transform with the object's
	sprite_.set_rotation(gef::DegToRad(transform_data_.rotation_.z()));  //...             
}

//constructor, requires standard game object stuff, needs only sprite renderer, sprite setup can hold texture or address to it, texture has priority
GameObject_2D::GameObject_2D(gef::Platform& platform, SpriteSetup setup, gef::SpriteRenderer* sprite_renderer) : GameObject(platform)
{
	if (!setup.texture) {                                       //if texture is not given in setup...
		gef::ImageData temp;
		gef::PNGLoader png_loader;
		png_loader.Load(setup.tex_address, platform_, temp);    //...load a new one from the address
		setup.texture = gef::Texture::Create(platform_, temp);  //...use the loaded data to create a texture
		texture_owner_ = true;                                  //becomes the texture owner, will delete it if this object gets deleted
	}

	sprite_renderer_ = sprite_renderer;                         //setup teh rest of the stuff using params
	sprite_.set_texture(setup.texture);                         //...
	sprite_.set_width(setup.width);
	sprite_.set_height(setup.height);

	transform_data_.position_ = Vector4(setup.posX, setup.posY, setup.ordZ);         
	texture_ = setup.texture;
}

//destructor override, combines with the inherited destructor 
GameObject_2D::~GameObject_2D()
{
	if (texture_ && texture_owner_) {         //if texture owner, deletes the texture
		delete texture_;
	}
}

//render override
void GameObject_2D::RenderObject()
{
	if (is_visible) {                           //renders using 2D stuff, conditions the render with the flag
		applyTransform();
		sprite_renderer_->DrawSprite(sprite_);
	}
}
