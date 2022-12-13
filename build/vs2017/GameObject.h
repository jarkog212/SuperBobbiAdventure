#pragma once

#include "TouchControls.h"
#include "Controls.h"
#include "KeyboardControls.h"
#include "audio_listener.h"
#include "audio_emitter.h"
#include "audio_3d.h"
#include <maths/math_utils.h>
#include <box2d/box2d.h>

using gef::Vector4;

//enum for the type of Rigid body, used to determine the dhape of the hit box

enum class RigidBody_Type {Box,Sphere};

//grouping of the transform values, vectors of position, rotation (euler), scale (DEG)
struct Transform{
	Vector4 position_ = Vector4(0.0f,0.0f,0.0f);
	Vector4 rotation_ = Vector4(0.0f,0.0f,0.0f);
	Vector4 scale_ = Vector4(1.0f, 1.0f, 1.0f);
};

//class b2Body;
//class b2World;
//class b2Vec2;
class Audio3D;
//enum b2BodyType;

namespace gef {

	class Platform;

}

//enum for object types, essential

enum ObjectType : int
{
BASE = 0,         //acts as undefined, or with sprites and stuff
PLAYER ,          //player object
FAUX_PLAYER,      //another multiplayer player
HAZZARD ,         //any harmful object to the player, only in level
ENVIRO,           //any non-harmful object, only in level
COLLECTIBLE       //any collectible
};

//base game object class, "alpha dog" or "the elder" class, children further spicify the behaviour

class GameObject
{
protected:

	Transform transform_data_;                                //transform data for the object and mesh, NOT physics body, gets updated by the phys body (if availble) every frame
	bool is_input_active_ = true;                             //flag for input
	bool is_responsive_collider_ = false;                     //flag for collision response function
	bool is_visible = true;                                   //flag for renderability
	ObjectType type_ = ObjectType::BASE;                      //value for the type

	class gef::Platform& platform_;                           //reference to the platform, needed for various things
	
	TouchControls* touch_input_ = NULL;                       //pointer to the touch control object, to be created dynamically
	Controls* controller_input_ = NULL;                       //...     ...the controller input object...
	KeyboardControls* keyboard_input_ = NULL;                 //...     ...the keyboard input object...
	
	b2World* phys_engine_ = NULL;                             //for cleanup of the body, used for proper deletion and memory management
	b2Body* phys_body_ = NULL;                                //physics body if needed, not all objects have them by default
	
	Audio3D* audio_3D_ = NULL;                                //audio 3D class, used to proliferate the audio manager as well as adding the audio emitter of this class          
	std::list<AudioEmitter*> emitters_;                       //array of looping emitters, memory management of audio 3D is handled in audio 3d hence non looping dont neet to be saved, this is needed to disable the looping 
	AudioListener* listener_ = NULL;                          //listener pointer, if this object ha sit enabled

	float temp_elapsed_time = 0.0f;                           //used in tandem with wait function, is only incremented by the frame time and only by wait funciton, gets reset once wait is finished

	void moveNoPhys(Vector4 translation);                       //changes the transform data's position by given translate
	void rotateNoPhys(Vector4 rotation);                        //...                       ...rotation by given rotation
	void scaleNoPhys(Vector4 scale);                            //...                       ...scale by given scale
	void moveWPhys(Vector4 translation);                        //changes the phys body's position by given translate, transform is then updated later
	void rotateWPhys(Vector4 rotation);                         //...                  ...Z rotation and strndard transform XY rotation by given rotation...
	void handleInput(float dT);                                 //handle input function, calls the apropriate sub functions depending on the availability of input
	virtual void handleControllerInput(float dT) {};            //virtual, specifies the behaviour of the controller if available
	virtual void handleTouchInput(float dT) {};                 //...                              ...the touch input...
	virtual void handleKeyboardInput(float dT) {};              //...                              ...the keyboard...
	virtual void update(float deltaT) {};                       //actual update function, safe to call, preceeded by checks
	virtual void applyPhysics();                                //applies the phys body transform into the transform_data
	virtual void applyTransform() {};                           //uses the transform_data to calculate appropriate coordinates for the object
	virtual void updateSound();                                 //specifies the behaviour of listener/emitter
	virtual void collisionResponse(GameObject* collider) {};    //collision response, preceeded by check hence safe to be called 
	void makePolygon(b2Vec2* points, int edges, float radius);  //used for building the phys body, when set in sphere mode, determines the number of polys
	bool wait(float &ms, float deltaT);                         //used when object needs to wait, partial (some functionality) and full (entire object)

public:
	GameObject(gef::Platform& platform);                        //no base constructor allowed as platform is required
	virtual ~GameObject();

	void Move(Vector4 translation);                                                                                                 //determines which move function to use (based on phys body availability)
	void Rotate(Vector4 rotation);                                                                                                  //...           ...rotate...
	void setPosition(Vector4 position);                                                                                             //...           ...move (with zero translate)...
	void setScale(Vector4 scale) { scaleNoPhys(scale); };                                                                           //just calls the scale function, no current phys body version, possible by unused
	void Update(float deltaT);                                                                                                      //calls relevant functions with correct checks every frame
	void CollisionResponse(GameObject* collider);                                                                                   //specifies the collision response
	void setupTouchInput(gef::InputManager* input) { touch_input_ = new TouchControls(input); }                                     //sets up the touch input, uses input manager
	void setupControllerInput(gef::InputManager* input) { controller_input_ = new Controls(input); }                                //...  ...the controller input...
	void setupKeyboardInput(gef::InputManager* input) { keyboard_input_ = new KeyboardControls(input); }                            //...  ...the keyboard...
	void setupListener(Audio3D* audio_3D);                                                                                          //sets up the listener, saves a pointer to audio 3D
	AudioEmitter* setupEmitter(Audio3D* audio_3D, int sfx_ID, bool looping, float radius = 1.0f);                                   //...  ...the emitter and if applicable, adds it to the vector
	void setupPhysics(b2World* phys_engine, RigidBody_Type phys_shape, b2BodyType type, float width, float height, float density);  //sets up th ephysics body, saves relevant pointers
	void toggleResponsiveCollider() { is_responsive_collider_ = !is_responsive_collider_; }                                         //toggle collision response
	void toggleInput() { is_input_active_ = !is_input_active_; }                                                                    //toggle input
	void toggleVisibility() { is_visible = !is_visible; }                                                                           //toggle visibility                                                                 
	virtual void RenderObject() {};                                                                                                 //allows for specific render rules

	//getters and setters
	ObjectType getType() { return type_; };
	void setObjectType(ObjectType type) { type_ = type; }
	void setVisibility(bool state) { is_visible = state; }
	bool getVisible() { return is_visible; }
	bool getResponsiveCollider() { return is_responsive_collider_; }
	bool getInputState() { return is_input_active_; }
	AudioListener* getListener() { return listener_; }
	b2Body* getBody() { return phys_body_; };
	const Transform getTransform() { return transform_data_; }
};

