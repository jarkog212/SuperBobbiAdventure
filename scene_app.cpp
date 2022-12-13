#pragma once
#include "scene_app.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/font.h>
#include <system/debug_log.h>
#include <graphics/renderer_3d.h>
#include <graphics/mesh.h>
#include <maths/math_utils.h>
#include <input/sony_controller_input_manager.h>
#include "SceneManager.h"
#include "Level1.h"
#include "PauseState.h"
#include "MainMenuState.h"
#include "MultiplayerLobbyState.h"
#include "GameOverState.h"
#include "SplashState.h"
#include "CreditsState.h"
#include "OptionsState.h"
#include "Globals.h"

//initialise the global variables
Globals globals;

SceneApp::SceneApp(gef::Platform& platform) :
	Application(platform),
	sprite_renderer_(NULL),
	renderer_3d_(NULL),
	primitive_builder_(NULL),
	input_manager_(NULL),
	audio_manager_(NULL),
	font_(NULL),
	sfx_id_(-1),
	audio_3d_(NULL)
{
}

//initialise global elements
void SceneApp::Init()
{
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);

	// create the renderer for draw 3D geometry
	renderer_3d_ = gef::Renderer3D::Create(platform_);

	// initialise primitive builder to make create some 3D geometry easier
	primitive_builder_ = new PrimitiveBuilder(platform_);

	// initialise input manager
	input_manager_ = gef::InputManager::Create(platform_);

	// initialise audio manager
	audio_manager_ = gef::AudioManager::Create();

	InitFont();
	
	// create 3d audio system
	audio_3d_ = new Audio3D(audio_manager_);

	InitSound();
	InitStateMachine();
}

//cleanup
void SceneApp::CleanUp()
{
	if (audio_3d_) {
		delete audio_3d_;
		audio_3d_ = NULL;
	}
	// free up audio assets
	if (audio_manager_)
	{
		audio_manager_->UnloadAllSamples();
	}

	CleanUpFont();

	if (audio_manager_) {
		delete audio_manager_;
		audio_manager_ = NULL;
	}

	if (input_manager_) {
		delete input_manager_;
		input_manager_ = NULL;
	}

	if (primitive_builder_) {
		delete primitive_builder_;
		primitive_builder_ = NULL;
	}

	if (renderer_3d_) {
		delete renderer_3d_;
		renderer_3d_ = NULL;
	}

	if (sprite_renderer_) {
		delete sprite_renderer_;
		sprite_renderer_ = NULL;
	}
}

//update all the parts of the program
bool SceneApp::Update(float frame_time)
{
	fps_ = 1.0f / frame_time;
	
	if (globals.G_button_down_timer > 0.00f) 
	{
		globals.G_button_down_timer -= frame_time;
	}
	
	input_manager_->Update();

	stateMachine_->Update(frame_time);
	// update 3d audio system
	audio_3d_->Update();

	return true;
}

//pass render call render
void SceneApp::Render()
{
	stateMachine_->Render();	
}

//initalise fonr
void SceneApp::InitFont()
{
	font_ = new gef::Font(platform_);
	font_->Load("pixeled");
}

//cleanup font
void SceneApp::CleanUpFont()
{
	delete font_;
	font_ = NULL;
}

//initialise all the kept states and the loader
void SceneApp::InitStateMachine()
{
	stateMachine_ = new SceneManager();
	stateMachine_->setupLoader(&platform_, font_,sprite_renderer_, renderer_3d_);

	PauseState* pause = new PauseState(&platform_, input_manager_, audio_manager_, font_, sprite_renderer_, renderer_3d_, audio_3d_);
	stateMachine_->addState(pause, GameState::PAUSE);

	MainMenuState* menu = new MainMenuState(&platform_, input_manager_, audio_manager_, font_, sprite_renderer_, renderer_3d_, audio_3d_);
	stateMachine_->addState(menu, GameState::MENU);

	GameOverState* go = new GameOverState(&platform_, input_manager_, audio_manager_, font_, sprite_renderer_, renderer_3d_);
	stateMachine_->addState(go, GameState::GAMEOVER);

	CreditsState* credits = new CreditsState(&platform_, input_manager_, audio_manager_, font_, sprite_renderer_, renderer_3d_);
	stateMachine_->addState(credits, GameState::CREDITS);

	OptionsState* options = new OptionsState(&platform_, input_manager_, audio_manager_, font_, sprite_renderer_, renderer_3d_);
	stateMachine_->addState(options, GameState::OPTIONS);
	
	MultiplayerLobbyState* multiplayerL = new MultiplayerLobbyState(&platform_, input_manager_, audio_manager_, font_, sprite_renderer_, renderer_3d_, audio_3d_);
	stateMachine_->addState(multiplayerL, GameState::MULTIPLAYER_LOBBY);

	SplashState* splash = new SplashState(&platform_, audio_manager_, sprite_renderer_, renderer_3d_);
	stateMachine_->setCurrentState(splash);

}

//create the global sfx map
void SceneApp::InitSound()
{
	if (audio_manager_)
	{
		audio_manager_->SetMusicVolumeInfo({ (float)globals.G_music_volume,0 });
		globals.SFX_map["Menu_move"] = audio_manager_->LoadSample("SFX/selector_movement.ogg", platform_);
		globals.SFX_map["Menu_select"] = audio_manager_->LoadSample("SFX/selector_select.ogg", platform_);
		globals.SFX_map["Player_boing"] = audio_manager_->LoadSample("SFX/boing.ogg", platform_);
		globals.SFX_map["Player_hit"] = audio_manager_->LoadSample("SFX/player_hit.ogg", platform_);
		globals.SFX_map["Collectible"] = audio_manager_->LoadSample("SFX/collectible.ogg", platform_);
		globals.SFX_map["Death"] = audio_manager_->LoadSample("SFX/death.ogg", platform_);
		globals.SFX_map["Win"] = audio_manager_->LoadSample("SFX/victory.ogg", platform_);
		globals.SFX_map["Blocky_move"] = audio_manager_->LoadSample("SFX/blocky_mov.ogg", platform_);
		globals.SFX_map["Blocky_boing"] = audio_manager_->LoadSample("SFX/blocky_boing.ogg", platform_);
		globals.SFX_map["Fan"] = audio_manager_->LoadSample("SFX/fan.ogg", platform_);
		globals.SFX_map["Pause"] = audio_manager_->LoadSample("SFX/pause.ogg", platform_);
	}
}
