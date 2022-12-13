#pragma once
#include "PauseState.h"

//options state, inherits from pause

class OptionsState :
    public PauseState
{
    void drawHUD() override;               //specific hud, hence override
public:
    OptionsState(gef::Platform* platform, gef::InputManager* input, gef::AudioManager* audio, gef::Font* font, gef::SpriteRenderer* sprite_renderer, gef::Renderer3D* renderer_3D) :
        PauseState(platform, input, audio, font, sprite_renderer, renderer_3D)
    {
        state_type_ = GameState::OPTIONS;  //type
    };
    void PlayStateBGM() override;          //override the bgm
    virtual void InitState() override;     //override cuz of different menu
};

