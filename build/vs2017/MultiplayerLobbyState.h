#pragma once
#include "PauseState.h"

//state for the multiplayer Lobby, uses pause state as base
class MultiplayerLobbyState :
    public PauseState
{
    SEND_TYPE_MESSAGE_FUNCTION_DECL(LoadLevel, std::string denominator)   //send function for LoadLevel message type 
    ON_MESSAGE_FUNCTION_DECLARATION(LoadLevel)                            //handler...

    void InitState() override;                                            //overrides  
    void update(float deltaT) override;                                   //...
    void drawHUD() override;              

    virtual void handleNetworking();                                      //networking update, receive and handle all messages

    bool canPlay_ = false;                                                //for server, whether there is at least  client connected
    std::string arenaDenominator_ = "A1";                                 //arena map layout to load, currently hardcoded but shuld be settable
    std::vector<GameObject_2D*> loadCircles_;                             //load circles used to show connection state
    Text ping_;                                                           //HUD element showing the internal value calculated for ping, debug

public:
    MultiplayerLobbyState(gef::Platform* platform, gef::InputManager* input, gef::AudioManager* audio, gef::Font* font, gef::SpriteRenderer* sprite_renderer, gef::Renderer3D* renderer_3D, Audio3D* audio_3D = NULL) :
        PauseState(platform, input, audio, font, sprite_renderer, renderer_3D, audio_3D)
    {
        state_type_ = GameState::MULTIPLAYER_LOBBY;    //sets the type to multiplayer lobby
    }
    void PlayStateBGM() override;                      //override for the music to be played the menu
};