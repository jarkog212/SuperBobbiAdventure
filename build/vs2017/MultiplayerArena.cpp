#pragma once
#include "MultiplayerArena.h"
#include <graphics/renderer_3d.h>
#include <system/platform.h>
#include "GameObject_2D.h"
#include <system/platform.h>
#include <graphics/scene.h>
#include <graphics/font.h>
#include <Globals.h>
#include "Camera.h"
#include "H_Blocky.h"
#include "H_Spike.h"
#include "Collectible.h"
#include "C_BigPebblet.h"
#include "C_Goal.h"
#include "FauxBobbi.h"
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
#define VICTORY_DELAY 7
#define DRAW_DELAY 15

#define SEND_RATE 1.00f/30.00f

//external link to global variables and network variables/objects
extern std::map<std::string, int> SFX_map;
extern Network network;
extern Globals globals;

void MultiplayerArena::handleNetworking(float frame_time)
{
	if (network.role == NetworkRole::Host)
		network.Server->ReceiveAllMessages();                 //if server, checkup whether server received any messages, if so sort them into queues
	else if (network.role == NetworkRole::Client)
		network.Client->ReceiveAllMessages();                 //if client...            ...client...

	//bool predictPhys = network.gotAnyObjectMessage();

	ALL_ON_MESSAGE_CALLS                                      //call all appropriate handlers for received messages

	//candidate for a separarte function - physics based prediction (BAD)
	/*if (predictPhys && network.role == NetworkRole::Client)
	{
		player_->getBody()->SetEnabled(false);
		phys_engine_->Step(max(network.Client->GetServerPing()/4 - frame_time, 0.0f), 4, 1);
		player_->getBody()->SetEnabled(true);
	}*/

		network.flushAllPublicMessages();                          //clean up just in case, no lingering messages as all should be handled
}

void MultiplayerArena::sendObject(GameObject_3D* toSend)
{
	ObjectMessage msg;                                                                                                                          //prepare a message to send, store the relevant data into it
	msg.M_objectID = toSend->GetNetworkID();                                                                                                    //...
	msg.M_health = toSend->getType() == ObjectType::PLAYER ? static_cast<PlayerBobbi*>(toSend)->getHits() : -1;
	msg.M_velocity = std::vector<double>{ toSend->getBody()->GetLinearVelocity().x, toSend->getBody()->GetLinearVelocity().y };
	msg.M_rotationZ = toSend->getBody()->GetAngularVelocity();
	msg.M_position = std::vector<double>{ toSend->getBody()->GetPosition().x, toSend->getBody()->GetPosition().y };
	msg.M_state = (toSend->getType() == ObjectType::PLAYER || toSend->getType() == ObjectType::FAUX_PLAYER) ? static_cast<PlayerBobbi*>(toSend)->getState() : 0;
	BuildObjectRawMessage(msg);                                                                    //serialize the data in the message to create a buffer to send

	if (network.role == NetworkRole::Host)
		network.Server->SendToAll(std::make_shared<Message>(msg));                                 //server send all messages to all clients
	else if (network.role == NetworkRole::Client)
		network.Client->SendMsg(&msg);                                                             //client only sends to server
}

void MultiplayerArena::onGameEndMessage(std::shared_ptr<Message>& msg)
{
	if (network.role == NetworkRole::Host)
		sendGameEnd();

	network.flushAllPublicMessages();
	state_switch_data_.new_kept_state_ = GameState::MULTIPLAYER_LOBBY;
	state_switch_data_.needs_to_change = true;
}

void MultiplayerArena::onObjectMessage(std::shared_ptr<ObjectMessage>& msg)
{
	std::list<GameObject_3D*>::iterator it = std::find_if(scene_objects_3D.begin(), scene_objects_3D.end(), [&](GameObject_3D* c) { //find the corresponding object in our scene to edit by received message...
		return (c->GetNetworkID() == msg->M_objectID && player_->GetNetworkID() != msg->M_objectID); });                            //...ignore messages about our player, onnly we simulate that

	if (it != scene_objects_3D.end())
	{
		if ((*it)->GetLastNetworkUpdate() < msg->M_timeStamp)                   //check whether this new message is more recent than the data stored, protects against reorders
		{
			auto newVelocity = b2Vec2(
				(float)msg->M_velocity[0],
				(float)msg->M_velocity[1]
			);

			//----------------------PREDICTION CODE - commented out because not in use
			/*float messageDeltaTime = (float)(msg->M_timeStamp - (*it)->GetLastNetworkUpdate()) / 1000.000f;
			b2Vec2 acceleration = averagedVelocity - (*it)->GetPreviousVelocity();
			acceleration.x /= messageDeltaTime;
			acceleration.y /= messageDeltaTime;*/

			//float ping = network.role == NetworkRole::Host ? network.Server->GetClientPing(msg->M_clientID) : network.Client->GetServerPing();

			/*if ((*it)->GetLastNetworkUpdate() == 0)
			{
				//acceleration = b2Vec2(0, 0);
				ping = 0;
			}*/

			(*it)->SetLastNetworkUpdate(msg->M_timeStamp);
			(*it)->getBody()->SetLinearVelocity(newVelocity);// + ping * acceleration);
			//(*it)->SetPreviousVelocity(newVelocity);

			(*it)->getBody()->SetAngularVelocity(msg->M_rotationZ);
			auto newPosition = Vector4(
				(float)(msg->M_position[0]), // + (*it)->getBody()->GetLinearVelocity().x * ping + 0.5 * acceleration.x * ping * ping),
				(float)(msg->M_position[1]), // + (*it)->getBody()->GetLinearVelocity().y * ping + 0.5 * acceleration.y * ping * ping),
				0, 0
			);

			(*it)->setPosition(newPosition);

			if ((*it)->getType() == ObjectType::FAUX_PLAYER)
			{
				static_cast<FauxBobbi*>((*it))->setState(msg->M_state); //only faux player object will be able to modify state
				static_cast<FauxBobbi*>((*it))->setHits(msg->M_health); //only faux player object will be able to modify hits
			}
		}
	}
}

void MultiplayerArena::onSpawnObjectMessage(std::shared_ptr<SpawnObjectMessage>& msg)                     //spawn an object based on the received message
{
	ObjectType typeToSpawn = (ObjectType)(msg->M_enum_major);                                             //deserialize object type
	switch (typeToSpawn)
	{
	case ObjectType::PLAYER:
		if (network.role == NetworkRole::Host) {                                  //server receives player spawn form client only to assign it a network Object ID...
			activePlayers_++;
			sendSpawnPlayer(msg->M_clientID);                                     //...resends this back to client so it can update the network ID...
			auto pos = Vector4(msg->M_position[0], msg->M_position[1], 0, 0);
			spawnFauxPlayer(pos, msg->M_clientID);                                //...sawns faux player to represent this player...
			sendSpawnFauxPlayer(pos, msg->M_clientID);                            //...sends this new faux player to other clients, even the one who owns the player - will be ignored there
		}
		else if (network.role == NetworkRole::Client)
			player_->SetNetworkID(msg->M_networkID);                              //client only uses this request to get the network id, player is already spawned
		break;

	case ObjectType::FAUX_PLAYER:                                                                  //faux player is used to represent other client/server players, only clients get this request
		if (network.role == NetworkRole::Client && player_->GetNetworkID() != msg->M_networkID)    //do not create a faux player for our own player
		{
			activePlayers_++;
			Vector4 pos = Vector4(msg->M_position[0], msg->M_position[1], 0, 0);
			spawnFauxPlayer(pos, msg->M_networkID);
		}
		break;

	case ObjectType::HAZZARD:
		Vector4 data = Vector4(msg->M_data[0], msg->M_data[1], msg->M_data[2], msg->M_data[3]);
		auto* temp = Level1::spawnHazzard(Vector4(msg->M_position[0], msg->M_position[1], 0, 0), (HazzardTypes)(msg->M_enum_minor), data); // the actual data does not matter as the object will be updated by UDP anyway

		if (network.role == NetworkRole::Host) {
			temp->SetNetworkID(nextID_);                                                                                                //server assigns the spawn object a new ID
			sendSpawnHazzard((HazzardTypes)msg->M_enum_minor, Vector4(msg->M_position[0], msg->M_position[1], 0, 0), data, nextID_);    //send the request to everyone with the ID
			nextID_++;                                                                                                                  //raises the nextID to ensure the new objects gets a unique one...
		}                                                                                                                               //...no decrements ever to ensure uniqueness without complex sorting
		else if (network.role == NetworkRole::Client)
			temp->SetNetworkID(msg->M_networkID);                                                                                       //simply assigns the spawned enemy received ID
		break;
	}
}

void MultiplayerArena::onClientReadyMessage(std::shared_ptr<Message>& msg)
{
	clientsReady_[msg->M_clientID] = true;
}

void MultiplayerArena::onPlayerLostMessage(std::shared_ptr<Message>& msg)
{
	if (network.role == NetworkRole::Host)
	{
		activePlayers_--;                              //server decreases the active player count
		clientsReady_[msg->M_clientID] = false;        //sets the client as not ready
		sendPlayerLost();                              //send the loss message to others
		if (countActivePlayers() <= 0)                 //if there are no aactive player, create fall back timer to cancel the game if draw, otherwise the winning client is responsible for ending the game
		{
			goal_timer_ = DRAW_DELAY;
			draw_ = true;
		}
	}
	else if (network.role == NetworkRole::Client)
		activePlayers_--;                              //if client, decrease active players only if confirmed by the server   
}

void MultiplayerArena::onStartGameMessage(std::shared_ptr<Message>& msg)
{
	if (network.role == NetworkRole::Client) {
		state_ = MultiplayerLevelState::COUNTDOWN;
		sendStartGame();                                                                  //sends back this message to server, to ensure it doesnt start before at least one client receives the message
	}

	else if (network.role == NetworkRole::Host && state_ == MultiplayerLevelState::PREPARE)
		state_ = MultiplayerLevelState::COUNTDOWN;                                        //as a server, only switch to countdown state once and only after receivig at least one confirmation from client
}

//------------------sender fucntions
void MultiplayerArena::sendSpawnHazzard(HazzardTypes type, Vector4 position, Vector4 data, int ID)
{
	SpawnObjectMessage msg;
	msg.M_enum_major = (int)ObjectType::HAZZARD;
	msg.M_enum_minor = (int)type;
	msg.M_position = { position.x(),position.y() };
	msg.M_data = std::vector<double>{ data.x(),data.y(),data.z(),data.w() };
	msg.M_networkID = ID;
	BuildSpawnObjectRawMessage(msg);

	if (network.role == NetworkRole::Host)
		network.Server->SendToAll(std::make_shared<Message>(msg));
	else if (network.role == NetworkRole::Client)
		network.Client->SendMsg(&msg);
}

void MultiplayerArena::sendSpawnPlayer(int clientID)
{
	//player is already spawned on the client, on the server only one info matters: the ID
	SpawnObjectMessage msg;
	msg.M_enum_major = (int)ObjectType::PLAYER;
	msg.M_enum_minor = 0;
	msg.M_position = { player_->getBody()->GetPosition().x, player_->getBody()->GetPosition().y };
	msg.M_data = std::vector<double>{ 0,0,0,0 };
	msg.M_networkID = clientID;
	BuildSpawnObjectRawMessage(msg);

	if (network.role == NetworkRole::Host)
		network.Server->SendMsgTo(&msg, clientID);
	else if (network.role == NetworkRole::Client)
		network.Client->SendMsg(&msg);
}

void MultiplayerArena::sendSpawnFauxPlayer(Vector4 position, int ID)
{
	//player is already spawned on the client, on the server only one info matters: the ID
	SpawnObjectMessage msg;
	msg.M_enum_major = (int)ObjectType::FAUX_PLAYER;
	msg.M_enum_minor = 0;
	msg.M_position = std::vector<double>{ position.x(),position.y() };
	msg.M_data = std::vector<double>{ 0,0,0,0 };
	msg.M_networkID = ID;
	BuildSpawnObjectRawMessage(msg);

	if (network.role == NetworkRole::Host)
		network.Server->SendToAll(std::make_shared<Message>(msg));
}

//map translating the Level block keys (strings) from files to enum
std::map<std::string, LevelBlockTypes> MultiplayerArena::setupMapForLevelBlocks()
{
	std::map<std::string, LevelBlockTypes> out;
	out["C1x1"] = LevelBlockTypes::C1x1;
	out["C1x2"] = LevelBlockTypes::C1x2;
	out["C13A"] = LevelBlockTypes::C13A;
	out["C13B"] = LevelBlockTypes::C13B;
	out["C2x3"] = LevelBlockTypes::C2x3;
	out["C4x4"] = LevelBlockTypes::C4x4;
	out["GC01"] = LevelBlockTypes::GC01;
	out["GC02"] = LevelBlockTypes::GC02;
	out["GC03"] = LevelBlockTypes::GC03;
	out["MGC1"] = LevelBlockTypes::MGC1;
	return out;
}

//init state
void MultiplayerArena::InitState()
{
	phys_engine_ = new b2World(b2Vec2(0, -9.8f));                         //create a phys engine

	if (models_.empty()) {                                                //load all the necessary models and textures into the array...
		loadModel("Models/BlueMechanical_4x4.scn");
		loadModel("Models/CityArena_1x1.scn");
		loadModel("Models/CityArena_1x2.scn");
		loadModel("Models/CityArena_1x3.scn");
		loadModel("Models/CityArena_1x32.scn");
		loadModel("Models/CityArena_2x3.scn");
		loadModel("Models/GroupCity1.scn");
		loadModel("Models/GroupCity2.scn");
		loadModel("Models/GroupCity3.scn");
		loadModel("Models/MegaGroupCity1.scn");
		loadModel("Models/Blocky.scn");
		loadModel("Models/Spike.scn");
	}

	std::string level_layout = "LevelData/";                              //prepare the level layout string address
	std::string enemy_list_file = "LevelData/";                           //...      ...enemy...

	std::string ammended;
	int atPos = level_denominator_.find('\0');                            //filter out the potential \0 char        
	if (atPos != std::string::npos)                                       //...
	{
		ammended.resize(atPos);
		memcpy((char*)ammended.data(), level_denominator_.data(), atPos);
	}
	else
		ammended = level_denominator_;

	level_layout.append(ammended);                                        //appends the address to match the required file name
	enemy_list_file.append(ammended);                                     //...

	level_layout.append(".txt");
	enemy_list_file.append("_E.txt");


	readLevelLayout(level_layout.data(), ObjectType::ENVIRO);
	if (network.role == NetworkRole::Host)
		readLevelLayout(enemy_list_file.data(), ObjectType::HAZZARD);     //spawn the objects based on the file, only server does this, clients wait for server spaw requests, needed for correct network id matching                 

	InitLight();                                                          //initiialise lights
	Init2DElements();                                                     //initialise the sprites and hud elements

	Vector4 playerPos = player_->getTransform().position_;                                                                                        //store player position
	Camera* camTemp = new Camera(renderer_3D_, { 0,0,0,0,0.1f,100.0f,45.0f,16.0f / 9.0f }, { Vector4(playerPos.x(),playerPos.y(),8),playerPos }); //create a camera
	SetupCamera(camTemp);                                                                                                                         //sets up the camera
}


//initialise 2D elements
void MultiplayerArena::Init2DElements()
{
	bg_address_ = "Textures/bg_multi.png";              //specify background stuff...
	bg_sprite_.set_uv_width(0.6);                       //...uv manipulation used for background scrolling
	bg_sprite_.set_uv_height(0.6);                      //...
	bg_sprite_.set_uv_position(gef::Vector2(0, 0.35));

	float width_ = platform_->width();       //save on constant memory access by saving the values locally, window width
	float height_ = platform_->height();     //...                                                              ...height

	goal_hud_ = setupSpriteObject(Vector4(width_ / 2, height_ / 2, -1), gef::Vector2(width_, height_), "Textures/multiWin.png");               //create the goal hud, save alone for render order
	goal_hud_->setVisibility(false);                                                                                                           //set to not visible
	fail_hud_ = setupSpriteObject(Vector4(width_ / 2, height_ / 2, -1), gef::Vector2(width_, height_), "Textures/multiLoss.png");              //create the lose hud, save alone for render order
	fail_hud_->setVisibility(false);                                                                                                           //set to not visible

	setupSpriteObject(Vector4(0, 0, -0.98), gef::Vector2(width_ / 2, width_ / 2), "Textures/flare.png");                                       //load the flare image, fake sun light
	GameObject_2D* temp_ = setupSpriteObject(Vector4(width_ / 2, -10, -0.99), gef::Vector2(width_, height_ / 8), "Textures/HUD.png");          //load the HUD banner and set it up
	temp_->Rotate(Vector4(0, 0, 180));
	hud_element_ = setupSpriteObject(Vector4(width_ / 2, height_ * 15.5 / 16, -0.99), gef::Vector2(width_, height_ / 8), temp_->getTexture()); //load the bottom hud, reuse the texture
	setupSpriteObject(Vector4(HITS_ICO_POS_X, HITS_ICO_POS_Y, -1), gef::Vector2(174, 149), "Textures/HitsIcon.png");                           //load the bandaid icon

	gef::Texture* temp_T = NULL;
	for (int i = 0; i < MAX_HITS_MULTI; ++i) {                                                                                                                 //create the right number of hit bars...
		if (i == 0)                                                                                                                                            //...with positions and proper...
			temp_ = setupSpriteObject(Vector4(HITS_ICO_POS_X - (166 + i * (153)), HITS_ICO_POS_Y + 35, -1), gef::Vector2(148, 86), "Textures/HitBarIcon.png"); //...offsets, automated
		else
			temp_ = setupSpriteObject(Vector4(HITS_ICO_POS_X - (166 + i * (153)), HITS_ICO_POS_Y + 35, -1), gef::Vector2(148, 86), temp_T);
		temp_T = temp_->getTexture();                                                                                                                      //reuse the texture
		hit_bars_.push_back(temp_);                                                                                                                        //push it into a single container
	}
}

//light setup
void MultiplayerArena::InitLight()
{
	// grab the data for the default shader used for rendering 3D geometry
	gef::Default3DShaderData& default_shader_data = renderer_3D_->default_shader_data();

	// set the ambient light
	default_shader_data.set_ambient_light_colour(gef::Colour(0.2f, 0.5f, 0.5f, 1.0f));

	// add a point light that is almost white, but with a blue tinge
	// the position of the light is set far away so it acts light a directional light
	gef::PointLight default_point_light;
	default_point_light.set_colour(gef::Colour(0.5, 1.0, 0.8, 1.0f));
	default_point_light.set_position(gef::Vector4(-200.0f, 400.0f, 700.0f));
	default_shader_data.AddPointLight(default_point_light);

	default_shader_data.AddPointLight(default_point_light);
}

//update - update for networking
void MultiplayerArena::update(float frame_time)
{
	handleNetworking(frame_time);                       //receive and handle all possible messages

	if (goalTimer_ > 0)                                  //used for draw timer
		goalTimer_ -= frame_time;

	switch (state_) {                                   //update state machine
	case MultiplayerLevelState::PREPARE:
		updatePrepare(frame_time);
		break;
	case MultiplayerLevelState::COUNTDOWN:
		state_ = MultiplayerLevelState::GAME;       //temp - needs to have proper countdown implemented
		audio_->PlayMusic();
		break;
	case MultiplayerLevelState::GAME:
		updateGame(frame_time);
		break;
	case MultiplayerLevelState::PLAYER_LOSE:
	default:
		break;
	}

	if (network.role == NetworkRole::Host)              //have server send all queued messages
		network.Server->SendMessagesToAll();
	else
	{
		network.Client->sendPingIn();                   //have client ask for ping
		//ping_ = { std::to_string(network.Client->GetServerPing()),gef::TJ_LEFT,Vector4(0,0,-1),3 }; //update the text object to display ping, used for debug
	}
}

void MultiplayerArena::updateGame(float frame_time)
{
	hold_in_memory_ = false;                                                                    //default level is not to be saved, resets any modification to this statement

	if (player_->getHits() <= 0 && !fail_hud_->getVisible())
		GameOver();                                                                             //checks and executes game over
	if (countActivePlayers() <= 1 && !fail_hud_->getVisible())
		Goal();                                                                                 //...              ...goal

	phys_engine_->Step(frame_time, 8, 3);                                                       //udates phys engine

	Vector4 playerPos = player_->getTransform().position_;                                      //makes the camera transform for the frame...
	if (playerPos.y() < CAMERA_MIN_Y)                                                           //...ensures camera does not dip below a certain level
		playerPos.set_y(CAMERA_MIN_Y);

	Vector4 delta_position_ = playerPos - camera_->getView().eye;                               //finds the difference between previous position and current position...
	gef::Vector2 bg_uv_movement_ = gef::Vector2(delta_position_.x(), delta_position_.y() * -1); //...to use it for camera uv manipulation...
	bg_sprite_.set_uv_position(bg_sprite_.uv_position() + bg_uv_movement_ * bg_mov_speed_);     //...to create a scrolling effect of the background

	camera_->setView({ Vector4(playerPos.x(),playerPos.y(),8),playerPos,Vector4(0,1,0) });      //sets the camera, happens after as there is a need for previous position for movement diff

	for (auto it : scene_objects_3D) {                                                                                                   //iterates through all 3D objects and updates them

		it->Update(frame_time);

		if ((it->getType() == ObjectType::PLAYER || it->getType() == ObjectType::HAZZARD || it->getType() == ObjectType::FAUX_PLAYER) &&  //if the object is a player, hazzard or False player and ...
			it->GetNetworkID() >= 0 &&                                                                                                    //...has an assigned valid network ID and...
			network.role == NetworkRole::Host &&                                                                                          //...we are a server and...
			sendTimer_ <= 0)                                                                                                              //...last send was correct time ago then...
		{
			sendObject(it);                                                                                                               //...make an object message and put it to queue
		}
		else if (it->getType() == ObjectType::PLAYER && network.role == NetworkRole::Client && sendTimer_ <= 0)                           //if the object is player and we are a client then...
			sendObject(it);                                                                                                               //...make an object message and send it
	}

	if (sendTimer_ <= 0)                                //update send timer
		sendTimer_ = SEND_RATE;

	sendTimer_ -= frame_time;                           //...

	for (auto it : scene_objects_2D)                    //iterates through all 2D objects and updates them
		it->Update(frame_time);
}

void MultiplayerArena::updatePrepare(float frame_time)
{
	if (!ready_)                                              //as an istance if were not ready, i.e. just loaded the level, the send the ready message
	{
		sendClientReady();
		ready_ = true;
	}

	if (network.role == NetworkRole::Host) {                  //server is only ready if connected clients are ready
		int canStart = 0;
		for (int i = 0; i < MAXCLIENTS; i++)
			clientsReady_[i] ? canStart++ : 0;

		if (canStart == network.Server->GetNumOfClients())
			sendStartGame();                                  //server sends the start game message
	}
}

void MultiplayerArena::updatePlayerLose(float frame_time)
{
	if (countActivePlayers() <= 1 && goalTimer_ < 0 && draw_)                  // check if all the players lost, if so check for draw and just in case end the game
		if (goalTimer_ < 0) {
			sendGameEnd();
			network.flushAllPublicMessages();
			state_switch_data_.new_kept_state_ = GameState::MULTIPLAYER_LOBBY;
			state_switch_data_.needs_to_change = true;
		}                                                                      //this function s uninvasive actually as server always needs to validate the game end, and in part send this to others...
}                                                                              //...and state change flushes all the messages anyways for anyone

void MultiplayerArena::updateHUD(float frame_time)
{
	for (int i = MAX_HITS_MULTI - 1; i >= 0; --i) {                        //iterates and shows only the correct amount of hit bars
		if (i + 1 > player_->getHits())
			hit_bars_[i]->setVisibility(false);
		else
			hit_bars_[i]->setVisibility(true);
	}

	if (goal_hud_->getVisible() || fail_hud_->getVisible()) {              //if state has changed from game, then disable game-related hud elements and enable the status hud
		for (auto it : scene_objects_2D) {
			if (it != goal_hud_ && it != fail_hud_) {
				it->setVisibility(false);
			}
		}
	}
}

//game over
void MultiplayerArena::GameOver()
{
	player_->getBody()->SetEnabled(false);           //disable player physics in current simulation, not done accross network

	state_ = MultiplayerLevelState::PLAYER_LOSE;     //change state nd inform server
	sendPlayerLost();

	if (network.role == NetworkRole::Host)           //host is not sending a message to itself hence manual subtraction
		activePlayers_--;

	fail_hud_->setVisibility(true);
}

//goal - update for networking
void MultiplayerArena::Goal()
{
	if (goalTimer_ < -0.9f) {                                              //float range taken into account
		player_->getBody()->SetEnabled(false);                            //disables the body, so that during the goal music it doesn't move
		goal_hud_->setVisibility(true);                                   //shows goal hud
		audio_->StopMusic();                                              //stops the music of the level, only the goal SFX plays
		audio_3D_->getAudioManager()->PlaySample(globals.SFX_map["Win"]); //...play the SFX for winning
		goalTimer_ = VICTORY_DELAY;                                       //continue iteratinf update while playing sfx, needed for networking
	}

	if (goalTimer_ < 0) {                                                   //after music plays, send game end and move back to lobby
		sendGameEnd();
		network.flushAllPublicMessages();
		state_switch_data_.new_kept_state_ = GameState::MULTIPLAYER_LOBBY;
		state_switch_data_.needs_to_change = true;
	}
}

GameObject_3D* MultiplayerArena::spawnPlayer(Vector4 position)
{
	player_ = new PlayerBobbi(*platform_, renderer_3D_, input_, phys_engine_, MAX_HITS_MULTI);           //create a player...

	if (network.role == NetworkRole::Client)                                              //as a client spawn player and inform the server to receive the right network ID
		sendSpawnPlayer(-1);
	else if (network.role == NetworkRole::Host)
	{
		sendSpawnFauxPlayer(position, MAXCLIENTS);                                       //.....server...                              ...assign the ID reserved for server
		player_->SetNetworkID(MAXCLIENTS);                                               //(0-(MAXCLIENTS-1) = other client players,  MAXCLIENTS = server player, (MAXCLIENTS+1)-inf = other objects)...
	}
	player_->placePlayer(position);                                                      //...setup its strarting transform...
	player_->setupListener(audio_3D_);                                                   //...add the audio 3D...
	addSceneObject(player_);                                                             //...push it into a container

	GameObject_3D* temp = new Fan(*platform_, input_, renderer_3D_, player_);            //create a fan to represent input, connect it to the player
	addSceneObject(temp);                                                                //push fan into the container
	return temp;
}

//---------------------------special spawner functions
GameObject_3D* MultiplayerArena::spawnHazzard(Vector4 position, HazzardTypes type, Vector4 data)
{
	int ID = -1;

	if (network.role == NetworkRole::Host) {
		auto temp = Level1::spawnHazzard(position, type, data);
		temp->SetNetworkID(nextID_);
		ID = nextID_;
		nextID_++;
	}
	sendSpawnHazzard(type, position, data, ID);
	return NULL;
}

FauxBobbi* MultiplayerArena::spawnFauxPlayer(Vector4 position, int ID)
{
	FauxBobbi* temp = new FauxBobbi(*platform_, renderer_3D_, phys_engine_);

	if (temp) {                           //if there is an object created, set up its transform an save it to the container
		temp->placePlayer(position);
		temp->SetNetworkID(ID);
		addSceneObject(temp);
		return temp;
	}
	return NULL;
}

int MultiplayerArena::countActivePlayers()
{
	return activePlayers_;
}

//base constructor, used when the previous state wasnt  level or a level with different assets, level denominator needs to be specified to correctly load the level
MultiplayerArena::MultiplayerArena(gef::Platform* platform, gef::InputManager* input, gef::AudioManager* audio, gef::Font* font, gef::SpriteRenderer* sprite_renderer, gef::Renderer3D* renderer_3D, Audio3D* audio_3D, std::string level_denominator) :
	Level1(platform, input, audio, font, sprite_renderer, renderer_3D, audio_3D, level_denominator)
{
}

//construtor with assets already loaded
MultiplayerArena::MultiplayerArena(LevelPack importPack) :
	MultiplayerArena(importPack.platform, importPack.input, importPack.audio, importPack.font, importPack.sprite_renderer, importPack.renderer_3D, importPack.audio_3D, importPack.next_lvl)
{
	models_ = importPack.models;
}

//override for level BGM, does not start playing the music as it waits for all clients to load
void MultiplayerArena::PlayStateBGM()
{
	hasBGM_ = true;
	audio_->LoadMusic("BGM/MultiplayerArena_CityBGM.wav", *platform_); //change to level theme
}
