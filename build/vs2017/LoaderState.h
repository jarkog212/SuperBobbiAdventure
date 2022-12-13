#pragma once
#include "State.h"
#include <thread>
#include <chrono>
#include <map>

using std::thread;
using std::map;

//special state used for loading, setup for it happen outside where it swithes itself into the current state

class LoaderState :
    public State
{
    map<GameState, State*>* kept_states_;      //pointer to the state machines saved states, states kept in memory
    State* previous_state_;                    //pointer to previous state, used for deletion purposes
    State* next_state_ = NULL;                 //pointer to the next state, used for loading
    State** P_currentState_;                   //pointer to the current state pointer
    GameState next_stored_state_;              //used if the next state is already loaded

public:
    LoaderState(gef::Platform* platform, State** current, gef::Font* font, gef::SpriteRenderer* sprite_renderer, gef::Renderer3D* renderer_3D, map<GameState, State*>* kept_states) :State(platform, NULL, NULL, font, sprite_renderer, renderer_3D,NULL,true) {
        P_currentState_ = current;
        kept_states_ = kept_states;
    };
    ~LoaderState();

    //override of StateRender might be needed
    void InitState() override;                                      //initialises the loader
    void StateUpdate(float frame_time) override;                    //where the loading happens, is not called every frame, instead called once and is stuck in until loading finishes
    void SetupLoading(State* previousState, GameState newState);    //setups the loading, saves previous state and next state, switches current to the loader
    void SetupLoading(State* previousState, State* nextState);      //...                             
    void ForceReleaseHolder();                                      //used when leaving pause into main menu, level wouldnt be deleted otherwise
};

