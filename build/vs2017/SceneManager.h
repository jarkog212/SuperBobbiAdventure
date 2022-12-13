#pragma once
#include <map>
#include <string>
#include "LoaderState.h"

using std::map;
using std::string;

//state machine, holds and delegates stuff regarding the states

class SceneManager
{
	map<GameState, State*> kept_states_;   //map of stored states, loaded once and then never delete
	GameState current_state_key_;          //current state key, used for manipulation of the stored states
	State* current_state_ = NULL;          //current state pointer
	LoaderState* loader_ = NULL;           //pointer to the loader state

public:
	SceneManager() {};
	~SceneManager();

	void Update(float frame_time);                                                                                                      //update that delegates
	void Render();                                                                                                                      //render...
	void checkCurrentState();                                                                                                           //checks whether the state wants to change to another
	void addState(State* state, GameState key);                                                                                         //adds to kept states
	void setupLoader(gef::Platform* platform, gef::Font* font, gef::SpriteRenderer* sprite_renderer, gef::Renderer3D* renderer_3D);     //sets up the loader
	void setCurrentState(GameState key);                                                                                                //sets the current state to a kept state
	void setCurrentState(State* state);                                                                                                 //...                      ...dynamic state
};

