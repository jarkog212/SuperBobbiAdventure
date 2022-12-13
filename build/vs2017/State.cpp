#pragma once
#include "LoaderState.h"
#include "GameObject_3D.h"
#include "GameObject_2D.h"
#include "Camera.h"
#include <graphics/sprite_renderer.h>
#include <graphics/font.h>
#include <graphics/renderer_3d.h>
#include <graphics/texture.h>
#include <graphics/image_data.h>
#include <assets/png_loader.h>
#include <system/platform.h>
#include "Globals.h"

//base update
void State::update(float frame_time)
{
	for (auto it : scene_objects_3D) {    //iterate through 3D container and update all the elements
		it->Update(frame_time);
	}
	for (auto it : scene_objects_2D) {    //iterate through 2D container and update all the elements
		it->Update(frame_time);
	}
}

//render 3D, iterate and render all 3D objects
void State::render_3D()
{
	for (auto it : scene_objects_3D) {
		it->RenderObject();
	}
}

//render 2D, iterate and render all 2D objects
void State::render_2D()
{
	for (auto it : scene_objects_2D) {
		it->RenderObject();
	}
}

//draw background
void State::drawBackground()
{
	if (!background_ && bg_address_ != "") {       //if there is no backgound texture yet and address is valid...
		SetupBackground();                         //...setup the background texture
	}
	if (background_) {                             //if the background texture is setup...
		sprite_renderer_->DrawSprite(bg_sprite_);  //...draw the background sprite
	}
}

//destructor
State::~State()
{
	for (auto it : scene_objects_3D) {
		if (it != NULL) {
			delete it;
			it = NULL;
		}
	}

	for (auto it : scene_objects_2D) {
		if (it != NULL) {
			delete it;
			it = NULL;
		}
	}

	if (phys_engine_) {
		delete phys_engine_;
		phys_engine_ = NULL;
	}

	if (camera_) {
		delete camera_;
		camera_ = NULL;
	}

	if (background_) {
		delete background_;
		background_ = NULL;
	}
}

//state update
void State::StateUpdate(float frame_time)
{
	if (input_) {           //if there is input, update it, probably not needed to update, as input is updated in scene app...
		input_->Update();
		handleInput();      //...call handle input
	}
	update(frame_time);     //call the virtual update
	collisionUpdate();      //...           ...collision update
	updateHUD(frame_time);  //...           ...update HUD
}

//render state
void State::StateRender()
{
	if (camera_) {                           //apply the camera if available
		camera_->apply();
	}
	if (sprite_renderer_) {                  //setup and render background
		sprite_renderer_->Begin(true);
		drawBackground();
		sprite_renderer_->End();
	}
	if (renderer_3D_) {                      //setup and render 3D objects
		renderer_3D_->Begin(false);
		render_3D();
		renderer_3D_->End();
	}
	if (sprite_renderer_) {                  //setup and render hud and 2D objects
		sprite_renderer_->Begin(false);
		drawHUD();
		render_2D();
		sprite_renderer_->End();
	}
}

//setup the background
void State::SetupBackground()
{
	gef::ImageData temp;
	gef::PNGLoader png_loader;
	png_loader.Load(bg_address_, *platform_, temp);
	background_ = gef::Texture::Create(*platform_, temp);

	bg_sprite_.set_texture(background_);                                         
	bg_sprite_.set_width(platform_->width());
	bg_sprite_.set_height(platform_->height());
	bg_sprite_.set_position(platform_->width() / 2, platform_->height() / 2, 2);
}

//sprite object creator, encapsulation
GameObject_2D* State::setupSpriteObject(Vector4 position, gef::Vector2 size, const char* tax_address)
{
	SpriteSetup temp_ = { position.x(),position.y(),position.z(),size.x,size.y,tax_address }; //create sprite setup
	GameObject_2D* object = new GameObject_2D(*platform_, temp_, sprite_renderer_);           //create a2D object using the sprite setup
	addSceneObject(object);                                                                   //push the sprite into the 2D container
	return object;                                                                            //returns a pointer to the created object
}

//sprite object creator, reuses texture, instead of loading a new one
GameObject_2D* State::setupSpriteObject(Vector4 position, gef::Vector2 size, gef::Texture* texture, bool fake)  
{
	SpriteSetup temp_ = { position.x(),position.y(),position.z(),size.x,size.y,"",texture };
	GameObject_2D* object = new GameObject_2D(*platform_, temp_, sprite_renderer_);
	addSceneObject(object);
	return object;
}

//update change of state, check whether this state wishes to change to a new one
void State::UpdateChangeOfState(LoaderState* LoaderState)
{
	if (state_switch_data_.needs_to_change) {                                   //if change flag is set...
		state_switch_data_.needs_to_change = false;                             //...disable it
		if (state_switch_data_.new_state_) {                                    //...if there is a dynamic state to switch to, call the loader with it
			LoaderState->SetupLoading(this, state_switch_data_.new_state_);
			state_switch_data_.new_state_ = NULL;                               //...prevent dangle
		}
		else {                                                                  //otherwise, call loder with the key
			LoaderState->SetupLoading(this, state_switch_data_.new_kept_state_);
		}
	}
}

//external link to the global variables
extern Globals globals;

//if possible, stop music
void State::StopAndResetStateAudio()
{
	if (audio_ && hasBGM_) {
		audio_->StopMusic();
		for (int i = 0; true; i++) {
			if (audio_->SetSampleVoiceVolumeInfo(i, { (float)globals.G_sfx_volume,100 }) < 0) {
				break;
			}
		}
	}
}
