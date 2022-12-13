#pragma once
#include "State.h"
#include <graphics/font.h>
#include "TouchControls.h"
#include "Controls.h"
#include "KeyboardControls.h"
#include "MenuWidget.h"
#include <graphics/sprite_renderer.h>


// grouping of the global pointers for lambda fucntions, easier to manage than captures
struct LambdaStruct {
    StateSwitcherData* switcher;           //pointer to state switcher
    gef::Platform* platform;               //...     ...global platform
    gef::InputManager* input;              //...            ...input manager
    gef::AudioManager* audio;              //...            ...audio manager
    gef::Font* font;                       //...            ...font
    gef::SpriteRenderer* sprite_renderer;  //...            ...sprite renderer
    gef::Renderer3D* renderer_3D;          //...            ...renderer 3D
    Audio3D* audio_3D;                     //...            ...audio 3D
};

//state for pause, used as a base for many static status based screens, ingherits from state

class PauseState :
    public State
{
protected:
    virtual void drawHUD() override;              //specific drawing instructions, hence override
    virtual void handleInput() override;          //handle input override, controller for menus

    TouchControls* touch_input_ = NULL;           //touch input object, to be created dynamically
    Controls* controller_input_ = NULL;           //Controller...
    KeyboardControls* keyboard_input_ = NULL;     //keyboard...
    MenuWidget<LambdaStruct> menu_;               //menu object, template uses lambda struct as the data type 

public:
    PauseState(gef::Platform* platform, gef::InputManager* input, gef::AudioManager* audio, gef::Font* font, gef::SpriteRenderer* sprite_renderer, gef::Renderer3D* renderer_3D, Audio3D* audio_3D = NULL) :
        State(platform, input, audio, font, sprite_renderer, renderer_3D, audio_3D, true) 
    {
        state_type_ = GameState::PAUSE;
    };
    virtual ~PauseState() {};

    virtual void InitState() override;                //init state, needs to be virtual for children to make their own menus

    //setup inputs, use input manager to create them
    void setupTouchInput() { touch_input_ = new TouchControls(input_); }   
    void setupControllerInput() { controller_input_ = new Controls(input_); }
    void setupKeyboardInput() { keyboard_input_ = new KeyboardControls(input_); }
    
};

