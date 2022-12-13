#pragma once
#include "PauseState.h"
#include "Camera.h"
#include <system/platform.h>

//defines for the menu position

#define MENU_POSITION_X 895
#define MENU_POSITION_Y 810

//init state override
void PauseState::InitState()
{
	bg_address_ = "Textures/pause.png";                               //sets the background
	menu_.position_ = Vector4(MENU_POSITION_X,MENU_POSITION_Y, 0);    //sets the menu position 
	menu_.audio_ = audio_;                                            //gives menu the audio manager

	setupControllerInput();                                           //sets up the input exclusive of the game object

	Text temp = {"CONTINUE",gef::TJ_LEFT,Vector4(0,0,-1),2};                                                                                              //menu element 1...
	menu_.AddElement(temp, [&](LambdaStruct struct_) {struct_.switcher->needs_to_change = true; struct_.switcher->new_kept_state_ = GameState::HOLDER; });//...switch to the holder, the level stored temporarily
	
	temp = { "RETURN TO MENU",gef::TJ_LEFT,Vector4(0,60,-1),2 };                                                                                          //menu element 2...
	menu_.AddElement(temp, [&](LambdaStruct struct_) {struct_.switcher->needs_to_change = true; struct_.switcher->new_kept_state_ = GameState::MENU; });  //...switch to the MENU state, loader will delete the stored level
}

//draw hud override
void PauseState::drawHUD()
{
	menu_.RenderMenu(sprite_renderer_, font_);
}

//handle input
void PauseState::handleInput()
{
	if (controller_input_->IsL2Held()) {
		if (globals.G_button_down_timer <= 0)
		{
			globals.G_debug_disable_controller = !globals.G_debug_disable_controller;
			globals.G_button_down_timer = BUTTON_DELAY;
		}
	}

	if (globals.G_debug_disable_controller)
		return;

	if (controller_input_->IsDPADDOWNHeld()) {         //if the DOWN button is pressed, next menu element will be selected
		menu_.Next();
	}
	else if (controller_input_->IsDPADUPHeld()) {      //if the UP button is pressed, previous menu element will be selected
		menu_.Previous();
	}

	if (controller_input_->IsCrossHeld()) {                                                                                  //if the X button is pressed...
		LambdaStruct struct_ = { &state_switch_data_,platform_,input_,audio_,font_,sprite_renderer_,renderer_3D_,audio_3D_};
		menu_.Select(struct_);                                                                                               //...execute the correct lambda function
	}
}