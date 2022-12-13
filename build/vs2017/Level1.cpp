#pragma once
#include "Level1.h"
#include <graphics/renderer_3d.h>
#include <system/platform.h>
#include "GameObject_2D.h"
#include <system/platform.h>
#include <graphics/scene.h>
#include <graphics/font.h>
#include "Camera.h"
#include "H_Blocky.h"
#include "H_Spike.h"
#include "Collectible.h"
#include "C_BigPebblet.h"
#include "C_Goal.h"
#include <iostream>
#include <iomanip>
#include <fstream>

//base defines, used for code clarity and sprite positions
#define IS_COLLIDER true
#define NOT_COLLIDER false

#define HEART_ICO_POS_X 100
#define HEART_ICO_POS_Y platform_->height() - 70

#define HITS_ICO_POS_X platform_->width() - 100
#define HITS_ICO_POS_Y platform_->height() - 70

#define CAMERA_MIN_Y 3

//external link to global variables
extern std::map<std::string, int> SFX_map;

//init state
void Level1::InitState()
{
	phys_engine_ = new b2World(b2Vec2(0, -9.8f));                 //create a phys engine

	if (models_.empty()) {                                        //load all the models into the array...
		loadModel("Models/Mechanical_1x1.scn");
		loadModel("Models/Mechanical_2x1.scn");
		loadModel("Models/Mechanical_3x1.scn");
		loadModel("Models/Mechanical_4x4.scn");
		loadModel("Models/Ground_2x1.scn");
		loadModel("Models/Ground_3x1.scn");
		loadModel("Models/Ground_4x1.scn");
		loadModel("Models/Ground_4x4.scn");
		loadModel("Models/MegaGroup1.scn");
		loadModel("Models/Group2.scn");
		loadModel("Models/Blocky.scn");
		loadModel("Models/Spike.scn");
		loadModel("Models/Pebblet.scn");
		loadModel("Models/Pebblet3.scn");
		loadModel("Models/Goal.scn");  
	}

	std::string level_layout = "LevelData/";                    //prepare the level layout string address
	std::string collectible_list_file = "LevelData/";           //...
	std::string enemy_list_file = "LevelData/";                 //...
	
	level_layout.append(level_denominator_);                        //appends the address to match the required file name
	collectible_list_file.append(level_denominator_);               //...
	enemy_list_file.append(level_denominator_);                     //...
	
	level_layout.append(".txt");                                    
	collectible_list_file.append("_C.txt");
	enemy_list_file.append("_E.txt");

	readLevelLayout(enemy_list_file.data(), ObjectType::HAZZARD);                //spawn the objects based on the file
	readLevelLayout(level_layout.data(), ObjectType::ENVIRO);                    //...
	readLevelLayout(collectible_list_file.data(), ObjectType::COLLECTIBLE);

	InitLight();              //initiialise lights
	Init2DElements();         //initialise the sprites and hud elements

	Vector4 playerPos = player_->getTransform().position_;                                                                                        //store player starting position
	Camera* camTemp = new Camera(renderer_3D_, { 0,0,0,0,0.1f,100.0f,45.0f,16.0f / 9.0f }, { Vector4(playerPos.x(),playerPos.y(),8),playerPos }); //create a camera
	SetupCamera(camTemp);                                                                                                                         //sets up the camera
}

//initialise 2D elements
void Level1::Init2DElements()
{
	bg_address_ = "Textures/bg.png";                    //specify background stuff...
	bg_sprite_.set_uv_width(0.6);                       //...uv manipulation used for background scrolling
	bg_sprite_.set_uv_height(0.6);                      //...
	bg_sprite_.set_uv_position(gef::Vector2(0, 0.35));

	float width_ = platform_->width();       //save on constant memory access by saving the values locally, window width, height
	float height_ = platform_->height();     //...

	goal_hud_ = setupSpriteObject(Vector4(width_/2, height_/2, -1), gef::Vector2(width_, 372), "Textures/goalOverlay.png");                    //create the goal hud, save alone for render order
	goal_hud_->setVisibility(false);                                                                                                           //set to not visible
	setupSpriteObject(Vector4(0, 0, -0.98), gef::Vector2(width_ / 2, width_ /2), "Textures/flare.png");                                        //load the flare image, fake sun light
	GameObject_2D* temp_ = setupSpriteObject(Vector4(width_ / 2, -10, -0.99), gef::Vector2(width_, height_ / 8), "Textures/HUD.png");          //load the HUD banner and set it up
	temp_->Rotate(Vector4(0, 0, 180));
	hud_element_ = setupSpriteObject(Vector4(width_ / 2, height_ * 15.5 / 16, -0.99), gef::Vector2(width_, height_ / 8), temp_->getTexture()); //load the bottom hud, reuse the texture
	setupSpriteObject(Vector4(HEART_ICO_POS_X, HEART_ICO_POS_Y, -1), gef::Vector2(155, 147), "Textures/LivesIcon.png");                        //load the heart icon
	setupSpriteObject(Vector4(HITS_ICO_POS_X, HITS_ICO_POS_Y, -1), gef::Vector2(174, 149), "Textures/HitsIcon.png");                           //load the bandaid icon

	gef::Texture* temp_T = NULL;                                                                                                                            
	for (int i = 0; i < MAX_HITS; ++i) {                                                                                                                   //create the right number of hit...
		if(i == 0)                                                                                                                                         //...bars with positions and proper...
			temp_ = setupSpriteObject(Vector4(HITS_ICO_POS_X - (166+ i*(153)), HITS_ICO_POS_Y + 35, -1), gef::Vector2(148, 86),"Textures/HitBarIcon.png"); //...offsets, automated
		else
			temp_ = setupSpriteObject(Vector4(HITS_ICO_POS_X - (166 + i * (153)), HITS_ICO_POS_Y + 35, -1), gef::Vector2(148, 86),temp_T);
		temp_T = temp_->getTexture();                                                                                                                      //reuse the texture
		hit_bars_.push_back(temp_);                                                                                                                        //push it into a single container
	}
	
}

//light setup
void Level1::InitLight()
{
	// grab the data for the default shader used for rendering 3D geometry
	gef::Default3DShaderData& default_shader_data = renderer_3D_->default_shader_data();

	// set the ambient light
	default_shader_data.set_ambient_light_colour(gef::Colour(0.7f, 0.1f, 0.0f, 1.0f));

	// add a point light that is almost white, but with a blue tinge
	// the position of the light is set far away so it acts light a directional light
	gef::PointLight default_point_light;
	default_point_light.set_colour(gef::Colour(1, 0.5, 0.3, 1.0f));
	default_point_light.set_position(gef::Vector4(-500.0f, 400.0f, 700.0f));
	default_shader_data.AddPointLight(default_point_light);

	default_shader_data.AddPointLight(default_point_light);
	
}

//render hud, uses 2D functions
void Level1::drawHUD()
{
	if (font_ && !goal_hud_->getVisible())                                                                                                //checks whether to render
	{ 
		hud_element_->RenderObject();                                                                                                     //render the bottom strip
		Text lives = {std::to_string(player_->getLives()).data(), gef::TJ_LEFT, Vector4(HEART_ICO_POS_X + 80 ,HEART_ICO_POS_Y - 20,-1) }; //make and render the lives text, set up params...
		lives.scale_ = 3;                                                                                                                 //...
		lives.displayText(sprite_renderer_, font_);

		Text score = { std::to_string(player_->getScore()), gef::TJ_CENTRE, Vector4(platform_->width()/2 ,HEART_ICO_POS_Y - 20,-1) };     //same text creation for score...
		score.scale_ = 3;                                                                                                                 //...
		score.displayText(sprite_renderer_, font_);
	}
}

//update
void Level1::update(float frame_time)
{
	hold_in_memory_ = false;                                                                    //default level is not to be saved, resets any modification to this statement
	if (player_->getLives() <= 0) { GameOver(); }                                               //checks and executes game over
	if (player_->getPingGoal()) { Goal(); }                                                     //...              ...goal

	phys_engine_->Step(frame_time, 8, 3);                                                       //udates phys engine

	Vector4 playerPos = player_->getTransform().position_;                                      //makes the camera transform for the frame...
	if (playerPos.y() < CAMERA_MIN_Y) {                                                         //...ensures camera does not dip below a certain level
		playerPos.set_y(CAMERA_MIN_Y);
	}
	
	Vector4 delta_position_ = playerPos - camera_->getView().eye;                               //finds the difference between previous position and current position...
	gef::Vector2 bg_uv_movement_ = gef::Vector2(delta_position_.x(), delta_position_.y()*-1);   //...to use it for camera uv manipulation...
	bg_sprite_.set_uv_position(bg_sprite_.uv_position() + bg_uv_movement_*bg_mov_speed_);       //... to create a scrolling effect of the background

	camera_->setView({ Vector4(playerPos.x(),playerPos.y(),8),playerPos,Vector4(0,1,0) });      //sets the camera, happens after as there is a need for previous position for movement diff

	for (auto it : scene_objects_3D) {                  //iterates through all 3D objects and updates them
		it->Update(frame_time);
	}	

	for (auto it : scene_objects_2D) {                  //iterates through all 2D objects and updates them
		it->Update(frame_time);
	}

	if (player_->getPingPause()) { Pause(); }           //checks whether to pause

	if (goal_hud_->getVisible()) {                      //checks if it should count the goal protocol...
		goal_timer_ += frame_time;
		if (goal_timer_ > 7.0f) {                       //...as it needs to wait for 7 seconds before...
			state_switch_data_.needs_to_change = true;  //...allowing the change to the next state
		}
	}
}

//updates the HUD
void Level1::updateHUD(float frame_time)
{
	for (int i = MAX_HITS - 1; i >= 0; --i) {   //iterates and shows only the correct amount of hit bars
		if (i + 1 > player_->getHits())
			hit_bars_[i]->setVisibility(false);
		else
			hit_bars_[i]->setVisibility(true);
	}

	if (goal_hud_->getVisible()) {              //if goal is visible, then disable every other hud element
		for (auto it : scene_objects_2D) {
			if (it != goal_hud_) {
				it->setVisibility(false);
			}
		}
	}
}

//collision update
void Level1::collisionUpdate()
{
	b2Contact* contact = phys_engine_->GetContactList();                                      //get all the collisions from the phys engine
	// get contact count
	int contact_count = phys_engine_->GetContactCount();                                      //since array return is impossible, manual memory jumping is needed, hence size of array is required

	for (int contact_num = 0; contact_num < contact_count; ++contact_num)                     //iterate through all collisions
	{
		if (contact->IsTouching())                                                            //if the objects collide
		{
			// get the colliding bodies
			b2Body* bodyA = contact->GetFixtureA()->GetBody();                                //get the phys bodies, save them 
			b2Body* bodyB = contact->GetFixtureB()->GetBody();                                //...

			GameObject* bA = reinterpret_cast<GameObject*>(bodyA->GetUserData().pointer);     //get the objects linked to bodies, translate them from pure ints to pointers of base GameObject
			GameObject* bB = reinterpret_cast<GameObject*>(bodyB->GetUserData().pointer);     //...
	        
			bA->CollisionResponse(bB);                                                        //call the collision response of the object and supply the collider
			bB->CollisionResponse(bA);                                                        //...
		}

		// Get next contact point
		contact = contact->GetNext();                                                         //continue to iterate through
	}
}

//pause
void Level1::Pause()
{
	hold_in_memory_ = true;                                      //temporarily allows for this state to be saved, just for returning from pause
	state_switch_data_.new_kept_state_ = GameState::PAUSE;       //sets up the swither to switch later to the pause state
	state_switch_data_.needs_to_change = true;                   //...
}

//game over
void Level1::GameOver()
{
	state_switch_data_.needs_to_change = true;                   //sets the switcher to switch to game over, deallocation is handled by the loader
	state_switch_data_.new_kept_state_ = GameState::GAMEOVER;
}

//goal 
void Level1::Goal()
{
	player_->getBody()->SetEnabled(false);                           //disables the body, so that during the goal music it doesn't move
	goal_hud_->setVisibility(true);                                  //shows goal hud
	audio_->StopMusic();                                             //stops the music of the level, only the goal SFX plays
	
	if (next_lvl_ == "") {                                           //if the next level file is not specified switches to credits
		state_switch_data_.new_kept_state_ = GameState::CREDITS;
	}
	else {
		state_switch_data_.new_state_ = new Level1(exportPack());    //if next level is specified, creates the husk of a level, gives it assets to reduce loading
	}
}

//loads the model in the scene form
void Level1::loadModel(const char* address)
{
	gef::Scene* scene = new gef::Scene();                           

	if (scene->ReadSceneFromFile(*platform_, address))               //reads the data from a file, if successful...
	{
		//...create material and mesh resources from the scene data
		scene->CreateMaterials(*platform_);
		scene->CreateMeshes(*platform_);
		models_[address] = scene;
	}
	else                //otherwise cleanup the scene and ensure no dangling pointer
	{
		delete scene;
		scene = NULL;
	}
}

//spawner for level block
void Level1::spawnLevelBlock(Vector4 position, Vector4 rotation, LevelBlockTypes type, bool is_collider)
{
	GameObject_3D* temp = new LevelBlock(*platform_, type, &models_, renderer_3D_, phys_engine_, is_collider);    //creates a new level block object with params...
	temp->setPosition(position);                                                                                  //...sets up its transform...
	temp->Rotate(rotation);  
	addSceneObject(temp);                                                                                         //...saves it to the object container
}

//spawner for hazzard
GameObject_3D* Level1::spawnHazzard(Vector4 position, HazzardTypes type, Vector4 data)
{
	GameObject_3D* temp = NULL;
	Vector4 initial_direction = Vector4(0, 1, 0, 0);                                                  //specifically for movable hazzards, initial direction

	switch (type) {                                                                                   //based on type create and setup the bhazzard object with params
	case HazzardTypes::BLOCKY:
		if (data.z() == 90 || data.z() == -270) { initial_direction = Vector4(-1, 0, 0, 0); }
		else if (data.z() == 180 || data.z() == -180) { initial_direction = Vector4(0,-1, 0, 0); }
		else if (data.z() == 270 || data.z() == -90) { initial_direction = Vector4(1, 0, 0, 0); }
		temp = new H_Blocky(*platform_, &models_, renderer_3D_, phys_engine_, initial_direction);
		temp->setupEmitter(audio_3D_, -1, false, 0.5f);                                               //fake emitter just for the audio 3D to be saved in the object
		break;
	case HazzardTypes::SPIKE:
		temp = new H_Spike(*platform_, &models_, renderer_3D_, phys_engine_, data);
		break;
	}

	if (temp) {                           //if there is an object created, set up its transform an save it to the container
		temp->setPosition(position);
		addSceneObject(temp);
		return temp;
	}
	return NULL;
}

//spawner for collec
GameObject_3D* Level1::spawnCollectible(Vector4 position, CollectibleTypes type)
{
	GameObject_3D* temp = NULL;

	switch (type) {                                                                  //based on type create a collectible and pass params
	case CollectibleTypes::PEBBLET:
		temp = new Collectible (*platform_, &models_, renderer_3D_, phys_engine_);
		break;
	case CollectibleTypes::BIG_PEBBLET:
		temp = new C_BigPebblet(*platform_, &models_, renderer_3D_, phys_engine_);
		break;
	case CollectibleTypes::GOAL:
		temp = new C_Goal(*platform_, &models_, renderer_3D_, phys_engine_);
		break;
	}

	if (temp) {                                         //if successful, set up its transform, add audio 3D and push it into the container
		temp->setPosition(position);
		temp->setupEmitter(audio_3D_, -1, false);
		addSceneObject(temp);
		return temp;
	}
	return NULL;
}

//spawner for player
GameObject_3D* Level1::spawnPlayer(Vector4 position)
{
	player_ = new PlayerBobbi(*platform_, renderer_3D_, input_, phys_engine_);           //create a player...
	player_->placePlayer(position);                                                      //...setup its strarting transform...
	player_->setupListener(audio_3D_);                                                   //...add the audio 3D...
	addSceneObject(player_);                                                             //...push it into a container

	GameObject_3D* temp = new Fan(*platform_, input_, renderer_3D_, player_);            //create a fan to represent input, connect it to the player
	addSceneObject(temp);                                                                //push fan into the container
	return temp;
}

//read the level form file
void Level1::readLevelLayout(const char* address, ObjectType Obj_type)
{
	std::ifstream inFile;
	char c;
	std::map<std::string, LevelBlockTypes> LB_translator_;   //creates an empty husk, used if loading pure layout
	std::map<std::string, HazzardTypes> H_translator_;       //...                                 ...hazzards
	std::map<std::string, CollectibleTypes> C_translator_;   //...                                 ...collectibles

	switch (Obj_type) {                                      //fill the map husk based on the type of file and objects we are loading, using helper functions
	case ObjectType::ENVIRO:
		LB_translator_ = setupMapForLevelBlocks();
		break;
	case ObjectType::HAZZARD:
		H_translator_ = setupMapForHazzards();
		break;
	case ObjectType::COLLECTIBLE:
		C_translator_ = setupMapForCollectibles();
		break;
	}

	/*
	           -FILE FORMAT-             -EXAMPLE-
	  l1:        NAME.xxx                 MGr2.002
	  l2:        posX                     15.005456825
	  l3:        posY                     2.5568225
	  l4:        rotZ                     180
	*/

	inFile.open(address);                                                 //open the level file
	if (!inFile) {                                                        //debug, deprecated
		std::cout << "Unable to open file";
		return; // terminate with error
	}

	if (Obj_type == ObjectType::ENVIRO) {                                 //if loading pure level layout...
		std::getline(inFile, next_lvl_);                                  //...first line contains a name of the next level
	}
	else {                                                                //otherwise ignore skip to next line
		inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	while (inFile.good()) {                                               //check whether there is data for the object, used to continue iterating thrugh the file till the end

		std::string temp; 
		std::string type;
		Vector4 position;
		Vector4 rotation;

		for (int i = 0; i < 4; i++) {                                      //next line's first 4 chars are the "name" of the object, its type key   
			inFile >> c;
			type.push_back(c);
		}
		inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  //skip to next line

		std::getline(inFile, temp);                                        //the next line contains x coordinate
		position.set_x(std::stof(temp));                                   //save the x coord into a vector
		temp.clear();                                                      //clear the string 
		std::getline(inFile, temp);                                        //the next line contains y coordinate
		position.set_y(std::stof(temp));                                   //save the y coord into a vector 
		position.set_z(0);                                                 //z must be 0, only 2D levels
		position.set_w(0);                                                 //useless
		std::getline(inFile, temp);                                        //next line contains z rotation in DEG
        rotation = Vector4(0, 0, std::stof(temp));                         //create a vector for rotation based on the value 
		temp.clear();

		if (type == "PLST") {                                              //"PLST" object key means the starting position of the player
			spawnPlayer(position);                                         //spawn the player
		}
		else {                                                                           //based on the file type, object type, spawn the corrct object
			switch (Obj_type) {                                                      
			case ObjectType::ENVIRO:
				spawnLevelBlock(position, rotation, LB_translator_[type], IS_COLLIDER);
				break;
			case ObjectType::HAZZARD:
				spawnHazzard(position, H_translator_[type], rotation);
				break;
			case ObjectType::COLLECTIBLE:
				spawnCollectible(position, C_translator_[type]);
				break;
			}
		}
		
	}
	inFile.close();                                                        //after all the bjects are loaded, close the file
}

//map translating the LEvel block keys from files to enum
std::map<std::string, LevelBlockTypes> Level1::setupMapForLevelBlocks()
{
	std::map<std::string, LevelBlockTypes> out;
	out["M1x1"] = LevelBlockTypes::M1x1;
	out["M2x1"] = LevelBlockTypes::M2x1;
	out["M3x1"] = LevelBlockTypes::M3x1;
	out["M4x4"] = LevelBlockTypes::M4x4;
	out["G2x1"] = LevelBlockTypes::G2x1;
	out["G3x1"] = LevelBlockTypes::G3x1;
	out["G4x1"] = LevelBlockTypes::G4x1;
	out["G4x4"] = LevelBlockTypes::G4x4;
	out["MGr1"] = LevelBlockTypes::MGR1;
	out["Grp2"] = LevelBlockTypes::GRP2;
	return out;
}

//map translating the hazzard keys from files to enum
std::map<std::string, HazzardTypes> Level1::setupMapForHazzards()
{
	std::map<std::string, HazzardTypes> out;
	out["BL11"] = HazzardTypes::BLOCKY;
	out["SPKS"] = HazzardTypes::SPIKE;
	out["SPIN"] = HazzardTypes::SPINNER;
	return out;
}

//map translating the collectibles keys from files to enum
std::map<std::string, CollectibleTypes> Level1::setupMapForCollectibles()
{
	std::map<std::string, CollectibleTypes> out;
	out["PBL1"] = CollectibleTypes::PEBBLET;
	out["PBL3"] = CollectibleTypes::BIG_PEBBLET;
	out["GOAL"] = CollectibleTypes::GOAL;
	return out;
}

//base constructor, used when the previous state wasnt  level or a level with different assets, level denominator needs to be specified to correctly load the level
Level1::Level1(gef::Platform* platform, gef::InputManager* input, gef::AudioManager* audio, gef::Font* font, gef::SpriteRenderer* sprite_renderer, gef::Renderer3D* renderer_3D, Audio3D* audio_3D, std::string level_denominator):
	State(platform,input,audio,font,sprite_renderer,renderer_3D,audio_3D)
{
	level_denominator_ = level_denominator;
}

//destructor
Level1::~Level1()
{
	gef::Default3DShaderData& default_shader_data = renderer_3D_->default_shader_data();
	default_shader_data.CleanUp();

	player_ = NULL; //potential error, used to ensure no dangling pointner
	
	if (assets_owner) {              //delets the assets if it owns them, ownership is taken away when exporting the assets, new level becomes the owner
		for (auto it : models_) {   
			delete it.second;
			it.second = NULL;
		}
	}
}

//export the level assets
LevelPack Level1::exportPack()
{
	LevelPack out{              //all ofthe global assets and level models are packed
		platform_,
		input_,
		audio_,
		font_,
		sprite_renderer_,
		renderer_3D_,
		audio_3D_,
		models_,
		next_lvl_
	};

	assets_owner = false;       //since exporting, no longer the owner of the assets, protets them from deletion
	return out;
}

//construtor with assets already loaded
Level1::Level1(LevelPack importPack):
	Level1(importPack.platform, importPack.input, importPack.audio, importPack.font, importPack.sprite_renderer, importPack.renderer_3D,importPack.audio_3D, importPack.next_lvl)
{
	models_ = importPack.models;
}

//override for level BGM
void Level1::PlayStateBGM()
{
	hasBGM_ = true;
	audio_->LoadMusic("BGM/Level1BGM.wav", *platform_);
	audio_->PlayMusic();
}
