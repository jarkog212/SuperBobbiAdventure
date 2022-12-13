#pragma once
#include "State.h"
#include <system/platform.h>

#define SPLASH_DURATION_S 5.2f  // intor in seconds

//splash screen with my logo, inherits form pause

class SplashState :
    public State
{
    float elapsed_time_ = 0.00f;                 //stopwatch
    void update(float deltaT) override;          //updates 
public:
    SplashState(gef::Platform* platform, gef::AudioManager* audio, gef::SpriteRenderer* sprite_renderer, gef::Renderer3D* renderer) :
        State(platform, NULL, audio, NULL, sprite_renderer, renderer) 
    {
        setupSpriteObject(gef::Vector4(platform->width() / 2, platform->height() / 2, 0), gef::Vector2(717, 336), "Textures/myLogo.png"); //setup the logo sprite
    }
    void PlayStateBGM() override;            //overrides to play intro BGM
    virtual void InitState() override {};    //disbale the pause init, not necessary
};

