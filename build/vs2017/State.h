#pragma once
#include <list>
#include "box2d/box2d.h"
#include <input/input_manager.h>
#include <graphics/image_data.h>
#include <graphics/sprite.h>
#include <graphics/font.h>

using std::list;

class GameObject;
class GameObject_3D;
class GameObject_2D;
class LoaderState;
class Camera;

namespace gef
{
	class Platform;
	class SpriteRenderer;
	class Font;
	class Renderer3D;
	class AudioManager;
	class Vector4;
	class Vector2;
}
class Audio3D;
class State;

//enum of the game states to be stored in the state machine
enum class GameState { MENU, OPTIONS, CREDITS, EXIT, GAMEOVER, WIN, LOADING, HOLDER, PAUSE , MULTIPLAYER_LOBBY};

//grouping of the text parameters
struct Text {
	std::string text_;                           //text string
	gef::TextJustification justification_;       //justification of the text
	gef::Vector4 position_;                      //position of the text
	float scale_ = 1.0f;                         //scale of the text
	int colour_ = (int)0xFFFFFFFF;               //colour of the text

	void displayText(gef::SpriteRenderer* sprite_renderer, gef::Font* font) {                             //render text
		font->RenderText(sprite_renderer, position_, scale_, colour_, justification_, text_.data());
	}
};

struct StateSwitcherData {
	GameState new_kept_state_;      //next state to switch to, used if the state is already kept
    State* new_state_ = NULL;       //...                                     ...is not yet loaded
	bool needs_to_change = false;   //flag to intiate the change next cycle
};

//base class for a state, member of the state machine

class State
{
protected:
	list<GameObject_3D*> scene_objects_3D;         //list of the 3D objects in the state
	list<GameObject_2D*> scene_objects_2D;         //list of the 2D objects in the state
	Camera* camera_ = NULL;                        //camera for the state
	gef::InputManager* input_ = NULL;              //pointer to global input manager
	gef::SpriteRenderer* sprite_renderer_ = NULL;  //...            ...sprite renderer
	gef::Renderer3D* renderer_3D_ = NULL;          //...            ...renderer 3D
	gef::Font* font_ = NULL;                       //...            ...font
	gef::AudioManager* audio_ = NULL;              //...            ...audio manager
	gef::Platform* platform_ = NULL;               //...            ...platform
	Audio3D* audio_3D_ = NULL;                     //...            ...audio 3D
	b2World* phys_engine_ = NULL;                  //...     ...physics engine, created by state dynamically

	gef::Texture* background_ = NULL;              //Texture to be used for background
	gef::Sprite bg_sprite_;                        //sprite for the background, not a game object
	const char* bg_address_ = "";                  //addres to the background texture
	GameState state_type_ = GameState::HOLDER;     //by default every state is holder, meaning it is not held in memory
	StateSwitcherData state_switch_data_;          //state switcher struct, checked every frame by th estate machine 
	bool hasBGM_ = false;                          //flag to notify loader whether to work with music

	virtual void handleInput() {};                 //handle input for the state, virtual
	virtual void update(float frame_time);         //update...              
	virtual void updateHUD(float frame_time) {};   //update HUD...
	virtual void render_3D();                      //render 3D objects of the state, with apropriate initialisation
	virtual void render_2D();                      //render 2D ...
	virtual void drawHUD() {};                     //draw HUD...
	virtual void collisionUpdate() {};             //collision update check for all objects within phys engine (if available)

	void drawBackground();                                                                                                //function to draw backgorund, first cycle also initialises it
	void SetupBackground();                                                                                               //called by drawBackground() first cycle
	GameObject_2D* setupSpriteObject(gef::Vector4 position, gef::Vector2 size, const char* tax_address);                  //simple encapsulation of othe initialisations for HUD sprites
	GameObject_2D* setupSpriteObject(gef::Vector4 position, gef::Vector2 size, gef::Texture* texture, bool fake = true);  //...
	bool hold_in_memory_ = false;                                                                                         //whether this state should be save din the map or not
public:
	State(gef::Platform* platform_ = NULL,gef::InputManager* input = NULL, gef::AudioManager* audio = NULL, gef::Font* font = NULL, gef::SpriteRenderer* sprite_renderer = NULL, gef::Renderer3D* renderer_3D = NULL, Audio3D* audio3D = NULL, bool hold_in_memory = false) :
		hold_in_memory_(hold_in_memory),
		platform_(platform_),
		sprite_renderer_(sprite_renderer),
		renderer_3D_(renderer_3D),
		font_(font),
		input_(input),
		camera_(NULL),
		audio_(audio),
		audio_3D_(audio3D)
	{};

	virtual ~State();
	virtual void InitState() {};                                                       //initialises the state, separate from constructor for loading purposes
	virtual void StateUpdate(float frame_time);                                        //calls relevant updates
    void StateRender();                                                                //...         ...renders
	virtual void UpdateChangeOfState(LoaderState* LoaderState);                        //checks whether the current state pinge for a change in state, if so setups the loader
	virtual void UpdateChangeOfState(GameState& currentState) {};                      //dont use, was without a loader if needed but needs to have all states saved, dangerous
	void addSceneObject(GameObject_3D* object) { scene_objects_3D.push_back(object); } //add scene object to the correct list, polymorphic
	void addSceneObject(GameObject_2D* object) { scene_objects_2D.push_back(object); } //... 
	
	//getters and setters
	virtual void SetupCamera(Camera* camera) { camera_ = camera; };
	void Set3DRenderer(gef::Renderer3D* renderer_3D) { renderer_3D_ = renderer_3D; }
	void SetSpriteRenderer(gef::SpriteRenderer* sprite_renderer) { sprite_renderer_ = sprite_renderer; }
	void SetInput(gef::InputManager* input) { input_ = input; }
	void SetFont(gef::Font* font) { font_ = font; }
	bool IsUndeleateable() { return hold_in_memory_; }
	GameState getType() { return state_type_; }
	
	
	virtual void PlayStateBGM() {};    //virtual function for the BGM, empty by default
	void StopAndResetStateAudio();     //stops BGM music, with appropriate checks
};

