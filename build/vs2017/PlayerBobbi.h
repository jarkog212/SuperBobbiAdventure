#pragma once
#include "GameObject_3D.h"
#include <string>
#include <system/string_id.h>

#define INVULNERABILITY_TIME 2           //seconds of invincibility after hit
#define MAX_HITS 3                       //max number of hits, period
#define MAX_HITS_MULTI 6                 //max number of hits, multiplayer

//class for the player, inherits form game object 3D

class PlayerBobbi :
    public GameObject_3D
{
protected:
    void handleControllerInput(float dt) override;             //player input is specific, hecne override
    void collisionResponse(GameObject* collider) override;     //collision response for the player
    void update(float dletaT) override;                        //player update
    void death();                                              //death of the player scenario

    int hits_ = 3;                           //num of hits
    int lives_ = 5;                          //num of lives 
    int score_ = 0;                          //score
    float timer_ = 0;                        //timer for invincibility
    float force_multiplier = 5.0f;           //for wind
    float elapsed_fan_time = 0.00f;          //time stopwatch used with wait()
    bool ping_pause_ = false;                //pings the state to pause
    bool ping_goal_ = false;                 //pings the state to run goal()
    int state_ = 0;                          //state of the player, chooses which testure to use
    Vector4 start_position_;                 //start position

    gef::StringId MainMaterialID;            //material id in scene/mesh for main texture
    gef::StringId FanMaterialID;             //...                        ...fan texture
    gef::StringId HurtMaterialID;            //...                        ...hurt texture

public:
    //change the mesh to const char for file asset instead of a rpimitive
    PlayerBobbi(gef::Platform& platform_, gef::Renderer3D* renderer, gef::InputManager* input, b2World* phys_engine, int hits = MAX_HITS);
    void placePlayer(Vector4 position);                                    //essentially setPosition but with more steps
    void reachedGoal() { ping_goal_ = true; is_input_active_ = false; }    //sets up the goal conditions

    //getters and setters
    void resetScore() { score_ = 0; } 
    int getHits() { return hits_; }
    int getLives() { return lives_; }
    int getScore() { return score_; }
    int getState() { return state_; }
    void addScore(int add) { score_ += add; }
    bool getPingPause();
    bool getPingGoal() { return ping_goal_; }
};

