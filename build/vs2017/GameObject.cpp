#pragma once
#include "GameObject.h"

//handle input
void GameObject::handleInput(float dT)
{
	if (is_input_active_) {                   //if input is active...
		if (touch_input_) {                   //and if touch input is available... 
			touch_input_->Update();           //...update and handle touch
			handleTouchInput(dT);
		}
		if (controller_input_) {              //if controller is available, update and handle it
			controller_input_->Update();
			handleControllerInput(dT);
		}
		if (keyboard_input_) {                //if keyboard is available...
			keyboard_input_->Update();
			handleKeyboardInput(dT);
		}
	}
}

//apply physics, sets the  phys transform data to the transform data for the object
void GameObject::applyPhysics()
{
	transform_data_.rotation_.set_z(gef::RadToDeg(phys_body_->GetAngle()));           //only z rotation

	transform_data_.position_.set_x(phys_body_->GetTransform().p.x);                  //only x and y
	transform_data_.position_.set_y(phys_body_->GetTransform().p.y);
}

//changes translation of the objects not inclusive of phys body
void GameObject::moveNoPhys(Vector4 translation)
{
	Vector4 movement_ = Vector4(translation.x(), translation.y(), translation.z());
	transform_data_.position_ += translation;
}

//adds to rotation of the objects not inclusive of phys body
void GameObject::rotateNoPhys(Vector4 rotation)
{
	transform_data_.rotation_ += rotation;
	transform_data_.rotation_.set_x(abs(transform_data_.rotation_.x()) > 360 ? transform_data_.rotation_.x() + 360 * (transform_data_.rotation_.x() > 0 ? -1 : 1) : transform_data_.rotation_.x());
	transform_data_.rotation_.set_y(abs(transform_data_.rotation_.y()) > 360 ? transform_data_.rotation_.y() + 360 * (transform_data_.rotation_.y() > 0 ? -1 : 1) : transform_data_.rotation_.y());
	transform_data_.rotation_.set_z(abs(transform_data_.rotation_.z()) > 360 ? transform_data_.rotation_.z() + 360 * (transform_data_.rotation_.z() > 0 ? -1 : 1) : transform_data_.rotation_.z());
}

//scales the body, should not be used if phys body available
void GameObject::scaleNoPhys(Vector4 scale)
{
	transform_data_.scale_ = scale;
}

//moves the phys body XY and mesh Z coord, this gets later reflected by the mesh
void GameObject::moveWPhys(Vector4 translation)
{
	transform_data_.position_.set_z(transform_data_.position_.z() + translation.z());                                       //objects Z
	b2Vec2 temp = b2Vec2(transform_data_.position_.x() + translation.x(), transform_data_.position_.y() + translation.y()); //vector for phys body, XY
	phys_body_->SetTransform(temp, phys_body_->GetAngle());                  
}

//add to rotation og phys body, Z only XY are mesh
void GameObject::rotateWPhys(Vector4 rotation)
{
	phys_body_->SetTransform(phys_body_->GetTransform().p, phys_body_->GetAngle() + gef::DegToRad(rotation.z()));
}

//updates audio 3d
void GameObject::updateSound()
{
		if (listener_) {                                                      //if this objects has/is a listener...
			gef::Matrix44 temp;
			gef::Matrix44 temp2;
			temp.SetIdentity();                                               //...build its transform matrix based on the transform data of the object
			temp2.SetIdentity();                                              //...

			temp.SetTranslation(transform_data_.position_);                   //...makes position (tranlsation) matrix

			temp2.RotationX(gef::DegToRad(transform_data_.rotation_.x()));    //...makes X axis rotation matrix
			temp = temp * temp2;                                              //...concatenates them (translate * rotationX)

			temp2.RotationY(gef::DegToRad(transform_data_.rotation_.y()));    //...makes Y axis rotation matrix 
			temp = temp * temp2;                                              //...concatenates them (previous result * rotation Y)

			temp2.RotationZ(gef::DegToRad(transform_data_.rotation_.z()));    //...makes Z axis rotation matrix
			temp = temp * temp2;                                              //...concatenates them (previous result * rotation Z)
			listener_->SetTransform(temp);                                    //...sends the result into the listener
		}
		if (!emitters_.empty()) {                                             //if this object has/is looping emiiters...
			for (auto it = emitters_.begin();it != emitters_.end(); it++) {   
				(*it)->set_position(transform_data_.position_);               //...setup its position relative to the listener, use the object's translate
				if (!(*it)->playing()) {                                      //...if the sfc is not playing,
					emitters_.erase(it);                                      //...clean up the emitter form memory, not dlete
				}
			}
		}
}

//builds a polygon mesh of vertices, using edges and radius, used for physics as a fixture
void GameObject::makePolygon(b2Vec2* points, int edges, float radius)
{
	if (edges > 8) { edges = 8; }
	float deltaAngle = 360 / edges;
	for (int i = 0; i < edges; i++) {
		points[i].x = radius * cos(deltaAngle*i);
		points[i].y = radius * sin(deltaAngle*i);
	}
	return;
}

//function for waiting
bool GameObject::wait(float &s, float deltaT = 0)
{
	if (temp_elapsed_time < s) {               //if the stopwatch is less than required wait, return true
		temp_elapsed_time += deltaT;           //add to stopwatch
		return true;                               
	}
	else {
		temp_elapsed_time = 0;                 //if the stopwatch is larger than the required wait...
		s = 0;                                 //...reset both to 0 and return stop
		return false;
	}
}

//constructor 
GameObject::GameObject(gef::Platform& platform) :
	platform_(platform)
{
}

//virtual destructor 
GameObject::~GameObject()
{
	//object owns touch input
	if (touch_input_) {              
		delete touch_input_;
		touch_input_ = NULL;
	}

	//objects owns controller
	if (controller_input_) {
		delete controller_input_;
		controller_input_ = NULL;
	}

	//objects owns keyboard
	if (keyboard_input_) {
		delete keyboard_input_;
		keyboard_input_ = NULL;
	}

	//object pseudo owns its phys body, destruction is handled by the world
	if (phys_body_) {
		phys_engine_->DestroyBody(phys_body_);
		phys_body_ = NULL;
	}

	//object pseudo owns its emmiters, only the looping ones, destruciton by audio 3D if set to non looping
	if (emitters_.size() != 0) {
		for (auto it = emitters_.begin(); it != emitters_.end(); it++) {
			(*it)->set_looping(false);
			emitters_.erase(it);
		}
	}
	
	//object pseudo owns listener, can delete it but dangling pointer in audio 3D needs to be fixed 
	if (listener_) {
		audio_3D_->deleteListener();
		delete listener_;
		listener_ = NULL;
	}
}

//public move, determines which move function to use
void GameObject::Move(Vector4 translation)
{
	if (phys_body_)
		moveWPhys(translation);
	else
		moveNoPhys(translation);
}

//public rotate, determines which rotate function to use 
void GameObject::Rotate(Vector4 rotation)
{
	if (phys_body_)
		rotateWPhys(rotation);
	else
		rotateNoPhys(rotation);
}

//public set position, sets the transform data position, forces the phys body to reflect that change as well
void GameObject::setPosition(Vector4 position)
{
	transform_data_.position_ = position;
	if (phys_body_)
		moveWPhys(Vector4(0, 0, 0, 0));
	else
		moveNoPhys(Vector4(0, 0, 0, 0));

}

//public update 
void GameObject::Update(float deltaT)
{
	handleInput(deltaT);    //call handle input delegate, checks happen within
	update(deltaT);         //calls virtual update
	if (phys_body_) {       
		applyPhysics();     //applies physics if available, matches phys body transform with object transform
	}
	updateSound();          //updates the sound, delegate
}

//collision response delegate with checks
void GameObject::CollisionResponse(GameObject* collider)
{
	if (phys_body_ && is_responsive_collider_) {
		collisionResponse(collider);
	}
}

//sets up the listener within audio 3d as well as this class
void GameObject::setupListener(Audio3D* audio_3D)
{
	if (!audio_3D_) {                   //ensures that it is not overwriting already saved audio 3d
		audio_3D_ = audio_3D;
	}
	listener_ = new AudioListener();    //creates audio listener dynamically
	audio_3D_->SetListener(listener_);  
}

//setup the emitter
AudioEmitter* GameObject::setupEmitter(Audio3D* audio_3D, int sfx_ID, bool looping, float radius)
{
	if (!audio_3D_) {                                       //saves but does not override the existing audio 3D
		audio_3D_ = audio_3D;
	}
	if (sfx_ID >= 0) {                                      //if legitimate sfx ID...
		AudioEmitter* emitter_ = new AudioEmitter();        //...create and initialise emitter
		emitter_->Init(sfx_ID, looping);
		emitter_->set_radius(radius);
		emitter_->set_position(transform_data_.position_);
		if (looping) {                                      //if emitter is looping...
			emitters_.push_back(emitter_);                  //...save it, non looping emitter is dealt with by audion 3D memory wise, for looping one, access needs to exist
		}
		audio_3D_->AddEmitter(emitter_);                    //push the emitter into audio 3D
		return emitter_;                                    //return a pointer to the emitter just in case
	}
}

//sets up the physics 
void GameObject::setupPhysics(b2World* phys_engine, RigidBody_Type phys_shape, b2BodyType type, float width, float height, float density)
{
	phys_engine_ = phys_engine;                                     // saves the pointer to the state's phys engine
	//add support fot future shapes
	b2BodyDef body;
	body.type = type;                                               //creates and sets up phys body attributes based on params
	body.position.Set(0.0f, 0.0f);
	body.userData.pointer = reinterpret_cast<uintptr_t>(this);      //saves a pointer to this object in a pure int form, needs to be recasted to be usable
	phys_body_ = phys_engine->CreateBody(&body);                    //adds the body to the phys engine (world) and returns its pointer, stored in the game object...
	                                                                //...body and object are linked in both directions            
	b2PolygonShape dynamic_box;                                     //creates a dynamic box shape defined further...

	switch (phys_shape) {                                           //...based on the type
		case RigidBody_Type::Box:
			dynamic_box.SetAsBox(width / 2, height / 2);            //box is simply taking the params and calling the function
			break;
		case RigidBody_Type::Sphere:                                //sphere requires the polygon vertices to be defined separately...
			b2Vec2 points[8];
			makePolygon(points, 8, width/2);                        //...hence a helper function is used/defined. determined resolution is 8 polygons - constant
			dynamic_box.Set(points, 8);                             
			break;
	}
	
	b2FixtureDef fixture_def;                                      //creates a fixture
	fixture_def.shape = &dynamic_box;                              //connects the shape with the fixture
	fixture_def.density = density;                                 //sets up density based on param, needed for dynamic mass

	phys_body_->CreateFixture(&fixture_def);                       //add fixture to the body
}
