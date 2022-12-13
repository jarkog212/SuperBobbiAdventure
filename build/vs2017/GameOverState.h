#pragma once
#include "PauseState.h"

//simple game over state, uses pause as base, shows picture, menu and text 

class GameOverState :
    public PauseState
{
    void InitState() override;                                       //different menu items require its own init function
public:
    GameOverState(gef::Platform* platform, gef::InputManager* input, gef::AudioManager* audio, gef::Font* font, gef::SpriteRenderer* sprite_renderer, gef::Renderer3D* renderer_3D) :
        PauseState(platform, input, audio, font, sprite_renderer, renderer_3D)
    {
        state_type_ = GameState::GAMEOVER;                           //sets the state for this to be kept in memory
    }
};

