#pragma once
#include "State.h"
#include "PlayerBobbi.h"
#include "Fan.h"
#include "LevelBlock.h"
#include <map>
#include <string>

enum class HazzardTypes;
enum class CollectibleTypes;


// struct that hold pointers and stuff, used to pass data form one level to another to reduce loading
struct LevelPack {
    gef::Platform* platform;                      //pointer to global platform
    gef::InputManager* input;                     //...            ...input manager
    gef::AudioManager* audio;                     //...            ...audio manager
    gef::Font* font;                              //...            ...font
    gef::SpriteRenderer* sprite_renderer;         //...            ...sprite renderer
    gef::Renderer3D* renderer_3D;                 //...            ...renderer3D
    Audio3D* audio_3D;                            //...            ...audio 3D
    std::map<std::string, gef::Scene*> models;    //container map of loaded scn, accessed by the file address as key
    std::string next_lvl = "";                    //used to determine which level file to read as next, if emtpy, goes to credits screen
};

//subclss of state, is going to be one per world, multiple levels can treuse this class without them being sublasses

class Level1 :
    public State
{
protected:
    std::map<std::string, gef::Scene*> models_;   //container map of loaded scn, accessed by the file address as key
    std::vector<GameObject_2D*> hit_bars_;        //used for the hud, sprites of the hit bars
    PlayerBobbi* player_;                         //pointer to the player, gets created dynamically by this class
    GameObject_2D* hud_element_;                  //pointer to the bottom hud banner (?) needed for render order of sprites
    GameObject_2D* goal_hud_;                     //pointer to the goal sprite, only visible once goal is hit
    
    float bg_mov_speed_ = 1.000f/500;             //movement speed of the background relative to the player movement speed
    Vector4 player_start_ = Vector4(-1, 1, 0);    //starting position of the player, used for restarting when dying
    std::string level_denominator_;               //current level characters, used to load level files by appending proper extensions to it
    std::string next_lvl_;                        //level denominator for next level
    float goal_timer_ = 0.0f;                     //time container for the goal grahic and music to stay on 
    bool assets_owner = true;                     //whether this level has the ability to delete the contaiiner of assets, if this is the last level of the world or game, then yes

    virtual void InitState() override;                     //actually loads the files and stuff, callled by the loader, separate form constructor
    virtual void Init2DElements();                         //...                                            ... init state, used for 2D elements
    virtual void drawHUD() override;                       //render function for the hud
    virtual void update(float frame_time) override;        //override of the update, renders the objects calls the relative render function in order: background, 3D then 2D
    virtual void updateHUD(float frame_time) override;     //update funciton for the hud, health update and score render
    virtual void collisionUpdate() override;               //calls the correct collisionresponses based on box 2D collision data
    virtual void InitLight();                              //intitialises the light
    
    virtual void Pause();                                                                                    //changes state to pause, temporarily stores the level to be switched back into later
    virtual void GameOver();                                                                                 //...           ...game over, loader deletes the level  
    virtual void Goal();                                                                             //...           ...next level, export assets and loader deletes the level

    void loadModel(const char* address);                                                               //loads the scene asset based on the address
    void spawnLevelBlock(Vector4 position, Vector4 rotation, LevelBlockTypes type, bool isCollider);   //spawner function for level block, combination of function and initialisation steps 
    virtual GameObject_3D* spawnHazzard(Vector4 position, HazzardTypes type, Vector4 data);                              //...               ...Hazzard...
    virtual GameObject_3D* spawnCollectible(Vector4 position, CollectibleTypes type);                                    //...               ...Collectible...
    virtual GameObject_3D* spawnPlayer(Vector4 position);                                                                //...               ...Player...
    void readLevelLayout(const char* address, ObjectType Obj_type);                                    //reads the file based on level denominator and type and spawns objects into the level
    virtual std::map<std::string, LevelBlockTypes> setupMapForLevelBlocks();                           //translates the level file object names into the object types, translator
    virtual std::map<std::string, HazzardTypes> setupMapForHazzards();                                 //...
    virtual std::map<std::string, CollectibleTypes> setupMapForCollectibles();                         //...

public:
    // constructors that setup for loading the level assets and for just connecting to the oaded assets
    Level1(gef::Platform* platform, gef::InputManager* input, gef::AudioManager* audio, gef::Font* font, gef::SpriteRenderer* sprite_renderer, gef::Renderer3D* renderer_3D, Audio3D* audio_3D = NULL, std::string level_denominator = "L1");
    Level1(LevelPack importPack);
    ~Level1();
    LevelPack exportPack();       //exports the level assets and stuff for the next level
    void PlayStateBGM() override; //override that determines the level music to play, called by the loader, hence public
};

