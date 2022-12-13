#pragma once
#include "PauseState.h"

//state class, child of paused state, shows menu and text

class CreditsState :
    public PauseState
{
    virtual void drawHUD() override;          //state only draws hud elements hence the override
public:
    CreditsState(gef::Platform* platform, gef::InputManager* input, gef::AudioManager* audio, gef::Font* font, gef::SpriteRenderer* sprite_renderer, gef::Renderer3D* renderer_3D) :
        PauseState(platform, input, audio, font, sprite_renderer, renderer_3D) 
    {
        state_type_ = GameState::CREDITS;     //sets the type to credits, essential if this is to be kept stored in memory                          
    }
    void InitState() override;                //separate from constructor for loading purposes
    void PlayStateBGM() override;             //overrideable class to play bgm, called by the loader to change the bgm, could be done better but this is fine
};

