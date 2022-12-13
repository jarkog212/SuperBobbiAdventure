#pragma once
#include "Level1.h"
#include "NetworkGlobals.h"

enum class HazzardTypes;
enum class CollectibleTypes;
class FauxBobbi;

extern Network network;

enum class MultiplayerLevelState : int {        //States of the multiplaer arena, results in calling different updates
    PREPARE = 0,
    COUNTDOWN,
    GAME,
    PLAYER_LOSE,
    RESULTS
};

class MultiplayerArena : 
    public Level1
{
protected:
    virtual void handleNetworking(float frame_time);       //general update for networking, receive and handle all the messages

    ALL_SEND_TYPE_MESSAGES                                 //declarations for all send functions for all message types
    ALL_ON_MESSAGE_FUNCTIONS_DECLARATIONS                  //...               ...handler...           

    void sendSpawnHazzard(HazzardTypes type, Vector4 position, Vector4 data, int ID = -1);                     //custom send spawn request functions...
    void sendSpawnPlayer(int clientID = -1);                                                                   //...
    void sendSpawnFauxPlayer(Vector4 position, int ID);

    virtual std::map<std::string, LevelBlockTypes> setupMapForLevelBlocks() override;                           //translates the level file object names into the object types, translator

    virtual void InitState() override;                     //actually loads the files and stuff, callled by the loader, separate form constructor
    virtual void Init2DElements();                         //...                                            ... init state, used for 2D elements
    virtual void InitLight();                              //intitialises the light
    virtual void update(float frame_time) final;           //override of the update, renders the objects calls network handler and respective update sub-function
    virtual void updateGame(float frame_time);             //update for when the game is playing
    virtual void updatePrepare(float frame_time);          //...              ...clients and server are preparing
    virtual void updateCountdown(float frame_time) {};     //...              ...game is about to start, not yet fully implemented - add number countdown
    virtual void updatePlayerLose(float frame_time);       //...              ...this instance of plaeyr has lost
    virtual void updateHUD(float frame_time) override;     //...     ...HUD elements
    void drawHUD() override {} //ping_.displayText(sprite_renderer_, font_);} //render all relevant HUD elements, not textures, those are considered scene objects

    virtual void GameOver() override;                                                                         //a function for game over, loader deletes the level  
    virtual void Goal() override;                                                                             //...         ...when a player wins, sends game nd and returns to lobby

    GameObject_3D* spawnPlayer(Vector4 position) override;                                                    //overrides for spawner functions...
    GameObject_3D* spawnHazzard(Vector4 position, HazzardTypes type, Vector4 data) override;                  //...
    FauxBobbi* spawnFauxPlayer(Vector4 position, int ID);

    int countActivePlayers();                                                    
    
    GameObject_2D* fail_hud_;                                        //pointer to fial hud, initialis in init function
    Text ping_;

    int nextID_ = MAXCLIENTS + 1;                                    //reserve the first MAXCLIENTS IDs for each clients players - incl. '0'. Server is MAXCLIENTS ID
    bool clientsReady_[MAXCLIENTS] = {false};                        //for server only
    int activePlayers_ = 1;                                          //number of active clients
    bool ready_ = false;                                             //readiness flag
    bool draw_ = false;                                              //whether the game ended in draw
    MultiplayerLevelState state_ = MultiplayerLevelState::PREPARE;   //current state of the level
    float goalTimer_ = -1.00f;                                       //timer for goal music
    float sendTimer_ = 0.00f;                                        //...    ...send function

public:
    // constructors that setup for loading the level assets and for just connecting to the oaded assets
    MultiplayerArena(gef::Platform* platform, gef::InputManager* input, gef::AudioManager* audio, gef::Font* font, gef::SpriteRenderer* sprite_renderer, gef::Renderer3D* renderer_3D, Audio3D* audio_3D = NULL, std::string level_denominator = "L1");
    MultiplayerArena(LevelPack importPack);
    void PlayStateBGM() override; //override that determines the level music to play, called by the loader, hence public
};



