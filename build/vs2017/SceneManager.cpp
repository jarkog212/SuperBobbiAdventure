#pragma once
#include "SceneManager.h"
#include "Globals.h"

extern Globals globals;

//destructor
SceneManager::~SceneManager()
{
	for (auto it : kept_states_) {
		delete it.second;
		it.second = NULL;
	}

	if (current_state_) {
		delete current_state_;
		current_state_ = NULL;
	}

	if (loader_) {
		delete loader_;
		loader_ = NULL;
	}
}

//update
void SceneManager::Update(float frame_time)
{
	if (current_state_) {
		checkCurrentState();                      //checks whether the current state wants to change into a different one
		current_state_->StateUpdate(frame_time);  //calls update
	}
}

//render
void SceneManager::Render()
{
	if (current_state_) {
		current_state_->StateRender();
	}
}

void SceneManager::checkCurrentState()
{
		if (!loader_) {                                                 //deprecated, loader is always present
			current_state_->UpdateChangeOfState(current_state_key_);
		}
		else {
			current_state_->UpdateChangeOfState(loader_);               //gives the current state access to loader for it to either switch a state or do nothing
		}
}

//add a state to kept states, one per key/per enum value
void SceneManager::addState(State* state, GameState key)
{
	if (state->IsUndeleateable()) {
		state->InitState();
		kept_states_[key] = state;
	}
}

//setup the loader, loader is special, handles memory of states, their dynamic allocation etc.
void SceneManager::setupLoader(gef::Platform* platform, gef::Font* font, gef::SpriteRenderer* sprite_renderer, gef::Renderer3D* renderer_3D)
{
	loader_ = new LoaderState(platform, &current_state_, font, sprite_renderer, renderer_3D, &kept_states_);
	loader_->InitState();
}

//set current state, uses key, hence can switch to only a kept state
void SceneManager::setCurrentState(GameState key)
{
	if (!loader_) {                                         //deprecated, loader is always present
		if (!current_state_->IsUndeleateable()) {
			delete current_state_;
		}
		current_state_key_ = key;
	}
	else {                                                 //sets up the loader to switch
		if (kept_states_[key]) {
			loader_->SetupLoading(current_state_, key);
			current_state_key_ = key;
			current_state_ = loader_;
		}
	}
}

//set current state, dynamic version
void SceneManager::setCurrentState(State* state)
{
	if (loader_) {                                    //setup loading with the state husk, needs initialisation
		loader_->SetupLoading(current_state_, state);
		current_state_ = loader_;
		current_state_key_ = GameState::LOADING;
	}
	else {                                            //deprecated and BAD
		if (!current_state_->IsUndeleateable()) {
			delete current_state_;
		}
		current_state_ = state;
		state->InitState();
		current_state_key_ = GameState::LOADING;
	}
}
