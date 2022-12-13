#pragma once
#include "CreditsState.h"
#include "system/platform.h"

//draw hud 
void CreditsState::drawHUD()
{
	Text credits = {"Congratulations!",gef::TJ_CENTRE,Vector4(platform_->width() / 2 ,30,-1),4 };            //creates and shows the text line  1
	credits.displayText(sprite_renderer_, font_);
	
	credits = { "You have beaten everything there s to beat in this demo...",gef::TJ_CENTRE,Vector4(platform_->width() / 2 ,150,-1),2 }; //creates and shows the text line  2
	credits.displayText(sprite_renderer_, font_);

	credits = { "I propose a trade, now that you have enjoyed exhilirating gameplay, gimme a good grade won't ya?",gef::TJ_CENTRE,Vector4(platform_->width() / 2 ,260,-1),1 }; //creates and shows the text line  3
	credits.displayText(sprite_renderer_, font_);
	
	menu_.RenderMenu(sprite_renderer_, font_);                                    //renders the menu
}

//init state
void CreditsState::InitState()
{
	menu_.position_ = Vector4((platform_->width()/2)-150, platform_->height()-400, 0);                 //set up menu position and audio
	menu_.audio_ = audio_;

	setupControllerInput();                                                                            //sets up the controller, uses the pause state funciton
	
	Text temp = { "RETURN TO MENU",gef::TJ_LEFT,Vector4(0,60,-1),2 };                                                                                     //menu  element 1...
	menu_.AddElement(temp, [&](LambdaStruct struct_) {struct_.switcher->needs_to_change = true; struct_.switcher->new_kept_state_ = GameState::MENU; });  //...goes to main menu
	temp = { "EXIT",gef::TJ_LEFT,Vector4(0,0,-1),2 };                                                                                                     //menu element 2...
	menu_.AddElement(temp, [&](LambdaStruct struct_) {exit(0); });                                                                                        //...closes the game

}

//play the bgm
void CreditsState::PlayStateBGM()
{
	hasBGM_ = true;
	audio_->LoadMusic("BGM/Credits.wav", *platform_);
	audio_->PlayMusic();
}

