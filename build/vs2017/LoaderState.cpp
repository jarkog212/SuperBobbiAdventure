#pragma once
#include "LoaderState.h"
#include <graphics/font.h>
#include <graphics/sprite_renderer.h>
#include <system/platform.h>
#include "scene_app.h""
#include "Camera.h"
#include "GameObject_2D.h"
#include <list>

//destructor 
LoaderState::~LoaderState()
{
	if (next_state_) {
		delete next_state_;        //ensures that next level is non existent, never used as loader doesn't get deleted
		next_state_ = NULL;
	}
}

//init loader
void LoaderState::InitState()
{
	//setup using helpers
	SpriteSetup twirl_setup = {platform_->width()*15 / 16, platform_->height() *15/16, -0.99,95,95,"Textures/LoadingTwirl.png"};  //creates the sprite setup for loading
	GameObject_2D* twirl = new GameObject_2D(*platform_, twirl_setup, sprite_renderer_);                                          //creates the sprite and pushes it into container
	addSceneObject(twirl);
}

//state update, special case where not executed once per frame but gets stuck here until loading is finished
void LoaderState::StateUpdate(float frame_time)
{
	bool finishedLoading = false;     //flag for loading
	bool finishedCleaning = false;    //flag for cleanup

	if (previous_state_) {                         //stop the BGM of previous state, if applicable is determied within the state
		previous_state_->StopAndResetStateAudio();
	}

	//THREAD 1 - CLEANING UP
	thread* cleanUp = new thread(
		[&] {
			if (previous_state_ && !previous_state_->IsUndeleateable()) {                                        //if prev state exists and is deleteable...
				delete previous_state_;                                                                          //...delete it
				previous_state_ = NULL;
			}

			if ((*kept_states_)[GameState::HOLDER] && !(*kept_states_)[GameState::HOLDER]->IsUndeleateable()) {  //if some state is temporarily stored and is deleateable...
				delete (*kept_states_)[GameState::HOLDER];                                                       //...delete it
				(*kept_states_)[GameState::HOLDER] = NULL;
			}

			if (previous_state_ && previous_state_->getType() == GameState::HOLDER)                              //if previous state exists,and it is a holder type...
			{                                                                                                    //...it got deleted already if it was deleteable...
				(*kept_states_)[GameState::HOLDER] = previous_state_;                                            //...store it temporarily
			}

			previous_state_ = NULL;             //no dangling pointers
			finishedCleaning = true;            //finished cleaning up
		}
	);

	//THREAD 2 - loading
	thread* loading = new thread(
		[&] {
			if (next_state_ == NULL) {                                     //if there is no unkept state to load into...
				//add selection of states based on game state type
				
				switch (next_stored_state_) {                              //...based on the kept state to load into... 
				case GameState::MENU:                                      //...in case of MENU...
					ForceReleaseHolder(); //needs error checking           //...forcibly delete HOLDER, level, made for PAUSE -> MENU transition...
					*P_currentState_ = (*kept_states_)[GameState::MENU];   //...switch to menu
					break;
				default:                                                   //by default...
					*P_currentState_ = (*kept_states_)[next_stored_state_];//...simply switch
					break;
				}
				
			}
			else {                                   //if there is an unkept state to switch into...
				*P_currentState_ = next_state_;      //...switch into it     
				next_state_ = NULL;                  //...undangle next state
				(*P_currentState_)->InitState();     //...properly initialise the state, for level loads the assets
			}
			(*P_currentState_)->PlayStateBGM();      //play the BGM
			finishedLoading = true;                  //finished loading
		}
	);
	
	//THREAD 3 - Render loading
	thread* loadingScreen = new thread(
		[&] {
			while (!finishedCleaning || !finishedLoading) {                                       //continuously render till the clean up and loading is not finished
				scene_objects_2D.front()->Rotate(Vector4(0, 0, 50 * platform_->GetFrameTime()));  //rotate the twirl 
				platform_->PreRender();                                                           //prepare for render
				StateRender();                                                                    //render the state
				platform_->PostRender();                                                          //buffer switch?
			}
		}
	);

	loading->join();          //join if finished
	cleanUp->join();          //join if finished
	loadingScreen->join();    //join if finished, dependent on the first two

}

//setup loading, uses state enum/key hence will load a kept state/ holder.
void LoaderState::SetupLoading(State* previousState, GameState nextStoredState)
{
	previous_state_ = previousState;          //saves previous state
	*P_currentState_ = this;                  //switches to loader to load
	next_stored_state_ = nextStoredState;     //saves the key for the state to load into 
	next_state_ = NULL;                       //we are not loading into an unkept state
}

//setup loading, uses state pointer, meaning we are loading into an unkept state
void LoaderState::SetupLoading(State* previousState, State* nextState)
{
	previous_state_ = previousState;          //saves the previous state
	*P_currentState_ = this;                  //switches to loader to load
	next_state_ = nextState;                  //saves the pointer to initialise and load into it later
}

//forcibly release a HOLDER, HOlder acts as a temporary saved state, used for pause
void LoaderState::ForceReleaseHolder()
{
	if ((*kept_states_)[GameState::HOLDER] && (*kept_states_)[GameState::HOLDER]->getType() == GameState::HOLDER) {   //if kept state is available and the type is holder...
		delete (*kept_states_)[GameState::HOLDER];                                                                    //...delete it
		(*kept_states_)[GameState::HOLDER] = NULL;
	}
}
