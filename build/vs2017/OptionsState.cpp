#pragma once
#include "OptionsState.h"
#include <system/platform.h>
#include "Globals.h"

#define MENU_POSITION_X platform_->width()/2
#define MENU_POSITION_Y platform_->height()/2 + 60

//external link to global variables
extern Globals globals;

//override init state
void OptionsState::InitState()
{
	bg_address_ = "Textures/Options.png";
	menu_.position_ = Vector4(MENU_POSITION_X, MENU_POSITION_Y, 0);  //set menu top left point to the middle of the screen
	menu_.audio_ = audio_;                                                     //pass audio to the menu

	setupControllerInput();                                                    //setup input, exclusive of the player

	Text temp = { "MUTE/UNMUTE",gef::TJ_RIGHT,Vector4(0,0,-1),2 };                                    //menu element 1...
	menu_.AddElement(temp, [&](LambdaStruct struct_) {                                                //...set the master volume to 100 or 0, global variable
		if (globals.G_master_volume > 0) { audio_->SetMasterVolume(0); globals.G_master_volume = 0; }
		else { audio_->SetMasterVolume(100); globals.G_master_volume = 100; }
		});

	temp = { "CHANGE MUSIC VOLUME",gef::TJ_RIGHT,Vector4(0,60,-1),2 };                                //menu element 2...
	menu_.AddElement(temp, [&](LambdaStruct struct_) {                                                //...add 10 to the Music volume, if over 100 set to 0, global variable
		globals.G_music_volume += 10;
		globals.G_music_volume %= 110;
		audio_->SetMusicVolumeInfo({ (float)globals.G_music_volume,0 });
		});

	temp = { "CHANGE SFX VOLUME",gef::TJ_RIGHT,Vector4(0,120,-1),2 };                                 //menu element 3...
	menu_.AddElement(temp, [&](LambdaStruct struct_) {                                                //...add 10 to SFX volume, if over 100 set to 0, global variable
		globals.G_sfx_volume += 10;
		globals.G_sfx_volume %= 110;
		audio_->SetSampleVoiceVolumeInfo(0, { (float)globals.G_sfx_volume,0 });
		});

	temp = { "INVERT Y AXIS",gef::TJ_RIGHT,Vector4(0,180,-1),2 };                                     //menu element 4...
	menu_.AddElement(temp, [&](LambdaStruct struct_) {                                                //...multiply the global invert y variable by -1 
		globals.G_invert_Y_axis *= -1;
		});

	temp = { "INVERT X AXIS",gef::TJ_RIGHT,Vector4(0,240,-1),2 };                                     //menu element 5...
	menu_.AddElement(temp, [&](LambdaStruct struct_) {                                                //...multiply the global invert x variable by -1
		globals.G_invert_X_axis *= -1;
		});

	temp = { "RETURN TO MENU",gef::TJ_CENTRE,Vector4(0,300,-1),2 };                                                                                      //menu element 6...
	menu_.AddElement(temp, [&](LambdaStruct struct_) {struct_.switcher->needs_to_change = true; struct_.switcher->new_kept_state_ = GameState::MENU; }); //...switch to MENU state
}

//draws the hud
void OptionsState::drawHUD()
{
	menu_.RenderMenu(sprite_renderer_, font_);                                                                     //renders the menu
	float X_position_ = (platform_->width() / 2) + 50;                                                             //gets the position of th e menu adds x offset of 50
	float Y_position_ = platform_->height() / 2;

	Text temp = { std::to_string(globals.G_music_volume),gef::TJ_LEFT,Vector4(X_position_,Y_position_+120,-1),2 };  //display global music volume value 
	temp.displayText(sprite_renderer_, font_);
	
	temp = { std::to_string(globals.G_sfx_volume),gef::TJ_LEFT,Vector4(X_position_,Y_position_ + 180,-1),2 };      //display global SFX volume value
	temp.displayText(sprite_renderer_, font_);

	std::string temp_string_ = "STANDARD";                                                   //Show "standard" if global invert Y variable is 1...
	if (globals.G_invert_Y_axis < 0) {                                                       //...show "Inverted" if -1
		temp_string_ = "INVERTED";
	}
	temp = { temp_string_,gef::TJ_LEFT,Vector4(X_position_,Y_position_ + 240,-1),2 };
	temp.displayText(sprite_renderer_, font_);

	temp_string_ = "STANDARD";                                                               //same as above except for inverted X
	if (globals.G_invert_X_axis < 0) {
		temp_string_ = "INVERTED";
	}
	temp = { temp_string_,gef::TJ_LEFT,Vector4(X_position_,Y_position_ + 300,-1),2 };
	temp.displayText(sprite_renderer_, font_);
}

//override for the BGM
void OptionsState::PlayStateBGM()
{
	hasBGM_ = true;
	audio_->LoadMusic("BGM/credits.wav", *platform_);
	audio_->PlayMusic();
}
