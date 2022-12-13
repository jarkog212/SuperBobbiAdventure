#pragma once
#include "PauseState.h"

//state for the main menu, uses pase satte as base

class MainMenuState :
    public PauseState
{
    void InitState() override;            //different menu and some textures, require an override for initState
    void update(float deltaT) override;   //update rotattes the star highligh
    void drawHUD() override;              //hud order is specific for menu hence override
    GameObject_2D* highlight_;            //pointer to the highligh, used to rotate
public:
    MainMenuState(gef::Platform* platform, gef::InputManager* input, gef::AudioManager* audio, gef::Font* font, gef::SpriteRenderer* sprite_renderer, gef::Renderer3D* renderer_3D, Audio3D* audio_3D = NULL) :
        PauseState(platform, input, audio, font, sprite_renderer, renderer_3D,audio_3D) 
    {
        state_type_ = GameState::MENU;    //stes the type to menu
    }
    void PlayStateBGM() override;         //override for the music to be played the menu
};

