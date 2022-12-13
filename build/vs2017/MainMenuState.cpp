#pragma once
#include "MainMenuState.h"
#include "Level1.h"
#include <system/platform.h>

//defines for sprite screen positions

#define MENU_POSITION_X 895
#define MENU_POSITION_Y 790

#define LOGO_POSITION_Y 400

//override initialise state
void MainMenuState::InitState()
{
	bg_address_ = "Textures/MenuBG.png";                              //sets the background texture address 
	menu_.position_ = Vector4(MENU_POSITION_X, MENU_POSITION_Y, 0);   //sets the menu params
	menu_.audio_ = audio_;

	setupControllerInput();                                           //creates a controller for input, exclusive of game object

	Text temp = { "PLAY",gef::TJ_LEFT,Vector4(0,0,-1),3 };            //menu element 1...
	menu_.AddElement(temp, [&](LambdaStruct struct_) {                //...create and stup loader to load a level (default denominator is "L1")
		struct_.switcher->needs_to_change = true;
		struct_.switcher->new_state_ = new Level1(struct_.platform, struct_.input, struct_.audio, struct_.font, struct_.sprite_renderer, struct_.renderer_3D, struct_.audio_3D);
		});
	
	temp = { "OPTIONS",gef::TJ_LEFT,Vector4(0,60,-1),3 };             //menu element 2...
	menu_.AddElement(temp, [&](LambdaStruct struct_) {                //...switch to a kept state OPTIONS
		struct_.switcher->needs_to_change = true;
		struct_.switcher->new_kept_state_ = GameState::OPTIONS;
		});

	temp = { "MULTIPLAYER",gef::TJ_LEFT,Vector4(0,120,-1),3 };             //menu element 3...
	menu_.AddElement(temp, [&](LambdaStruct struct_) {                //...switch to a kept state MULTIPLAYER_LOBBY
		struct_.switcher->needs_to_change = true;
		struct_.switcher->new_kept_state_ = GameState::MULTIPLAYER_LOBBY;
		});

	temp = { "EXIT",gef::TJ_LEFT,Vector4(0,180,-1),3 };               //menu element 4...
	menu_.AddElement(temp, [&](LambdaStruct struct_) {exit(0); });    //...quit the game

	highlight_ = setupSpriteObject(Vector4(platform_->width() / 2, LOGO_POSITION_Y, -0.98), gef::Vector2(1062, 1062), "Textures/highlight.png"); //create a highlight sprite setup
	setupSpriteObject(Vector4(platform_->width() / 2, LOGO_POSITION_Y, -0.99), gef::Vector2(962, 535), "Textures/TitleLogo.png");                //create and push the sprite
}

//override draw HUD specific
void MainMenuState::drawHUD()
{
	highlight_->RenderObject();                //render the behind the logo highlight
	menu_.RenderMenu(sprite_renderer_, font_); //render the menu
	
}

//override play the menu bgm
void MainMenuState::PlayStateBGM()
{
	hasBGM_ = true;
	audio_->LoadMusic("BGM/MainMenuBGM.wav", *platform_);
	audio_->PlayMusic();
}

//override update 
void MainMenuState::update(float deltaT)
{
	highlight_->Rotate(Vector4(0, 0, 30 * deltaT));  //rotate the highlight
}
