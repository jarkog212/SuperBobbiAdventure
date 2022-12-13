#pragma once
#include "GameOverState.h"

//defines for sprite positions
#define MENU_POSITION_X 895
#define MENU_POSITION_Y 810

//init state
void GameOverState::InitState()
{
	bg_address_ = "Textures/gameover.png";                               //sets the background image
	menu_.position_ = Vector4(MENU_POSITION_X, MENU_POSITION_Y, 0);      //sets the position of the entire menu
	menu_.audio_ = audio_;                                               //connects the audio manager with the menu

	setupControllerInput();                                              //separate controller for menu, exclusive of the object

	Text temp = { "BACK TO MENU",gef::TJ_LEFT,Vector4(0,0,0),2 };        //menu element 1...
	menu_.AddElement(temp, [&](LambdaStruct struct_) {                   
		struct_.switcher->needs_to_change = true;
		struct_.switcher->new_kept_state_ = GameState::MENU; });         //...switch state to main menu
	temp = { "EXIT",gef::TJ_LEFT,Vector4(0,60,0),2 };                    //menu element 2...
	menu_.AddElement(temp, [&](LambdaStruct struct_) {exit(0); });       //...quit the game
}
