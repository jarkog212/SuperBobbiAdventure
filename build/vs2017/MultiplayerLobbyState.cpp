#include "MultiplayerLobbyState.h"
#include "MultiplayerArena.h"
#include <system/platform.h>
#include "Globals.h"

//defines for sprite screen positions

#define MENU_POSITION_X 895
#define MENU_POSITION_Y 810

#define LOGO_POSITION_Y 400

//networking externals
extern Network network;

//override initialise state
void MultiplayerLobbyState::InitState()
{
	bg_address_ = "Textures/multiLobby.png";                              
	menu_.position_ = Vector4(MENU_POSITION_X, MENU_POSITION_Y, 0);   
	menu_.audio_ = audio_;
	menu_.ChangeBaseColour((int)0xFF808080);

	setupControllerInput();                                           

	Text temp = { "HOST",gef::TJ_LEFT,Vector4(0,0,-1),3 };                    //Menu option "HOST"  
	menu_.AddElement(temp, [&](LambdaStruct struct_) {                
		NetworkServer::CreateServer();                                        //create a server object, should fail if innapplicable
		menu_.RemoveElement("HOST");                                          //remove menu options
		menu_.RemoveElement("CONNECT");                                       //...
		});

	temp = { "CONNECT",gef::TJ_LEFT,Vector4(0,60,-1),3 };                     //Menu option "CONNECT"
	menu_.AddElement(temp, [&](LambdaStruct struct_) {                        //creates a client object, automatically tries to connect
		NetworkClient::CreateClient();
	});

	temp = { "BACK",gef::TJ_LEFT,Vector4(0,120,-1),3 };                       //Menu option "BACK"
	menu_.AddElement(temp, [&](LambdaStruct struct_) {                        //cleans up what needs to be cleaned up and returns to main menu
		if (network.role == NetworkRole::Client)
			network.Client->cleanUpClient();
		if (network.role == NetworkRole::Host)
			network.Server->CleanUpServer();
		struct_.switcher->needs_to_change = true;
		struct_.switcher->new_kept_state_ = GameState::MENU; 

		menu_.RemoveElement("HOST");
		menu_.RemoveElement("CONNECT");
		menu_.RemoveElement("PLAY");
		menu_.RemoveElement("BACK");

		InitState();                                                          //re-initialises the state for future use
	});    

	if (loadCircles_.empty())                                                 //initialise the loading circles if not already there
	{
		loadCircles_.push_back(setupSpriteObject(Vector4(platform_->width() / 2, platform_->height() / 2, -0.99), gef::Vector2(300, 300), "Textures/loadCircle.png"));
		loadCircles_.push_back(setupSpriteObject(Vector4(platform_->width() / 2, platform_->height() / 2, -0.99), gef::Vector2(200, 200), loadCircles_.back()->getTexture()));
		loadCircles_.push_back(setupSpriteObject(Vector4(platform_->width() / 2, platform_->height() / 2, -0.99), gef::Vector2(100, 100), loadCircles_.back()->getTexture()));
	}
}

//override draw HUD specific
void MultiplayerLobbyState::drawHUD()
{
	menu_.RenderMenu(sprite_renderer_, font_);     //render the menu
	ping_.displayText(sprite_renderer_, font_);    //render ping 
}

void MultiplayerLobbyState::handleNetworking()
{
	if (network.role == NetworkRole::Host)
		network.Server->ReceiveAllMessages();
	else if (network.role == NetworkRole::Client)
		network.Client->ReceiveAllMessages();
	else
		return;

	CALL_MESSAGE_TYPE_HANDLER_FUNCTION(LoadLevel)  //check if any LoadLevel messages and if so, handle them

	network.flushAllPublicMessages();              //cleanup just in case
}

//override play the menu bgm
void MultiplayerLobbyState::PlayStateBGM()
{
	hasBGM_ = true;
	audio_->LoadMusic("BGM/MultiplayerLobby.wav", *platform_);
	audio_->PlayMusic();
}

void MultiplayerLobbyState::sendLoadLevel(std::string denominator)
{
	LoadLevelMessage msg;
	msg.M_level_denominator = arenaDenominator_;                              //sends the current denominator for the client to use to load the level
	BuildLoadLevelRawMessage(msg);

	if (network.role == NetworkRole::Host)
	{
		network.Server->SendToAll(std::make_shared<LoadLevelMessage>(msg));
		network.Server->SendMessagesToAll();
	}
}

void MultiplayerLobbyState::onLoadLevelMessage(std::shared_ptr<LoadLevelMessage>& msg) 
{
	if (state_switch_data_.new_state_ && !state_switch_data_.new_state_->IsUndeleateable())
		delete state_switch_data_.new_state_;

	state_switch_data_.new_state_ = new MultiplayerArena(platform_, input_, audio_, font_, sprite_renderer_, renderer_3D_, audio_3D_, msg->M_level_denominator);  //tries to load the received level denominator
	state_switch_data_.needs_to_change = true;                                                                                                                    //flags the switch
}

//override update 
void MultiplayerLobbyState::update(float deltaT)
{
	handleNetworking();                                                  //handle networking, receive and handle messages
	
	if (network.role == NetworkRole::Host) 
	{
		if (network.Server->GetNumOfClients() > 0 && !canPlay_) {        //server check if any players are connected and if so allows the game to be started by the player....

			Text temp = { "PLAY",gef::TJ_LEFT,Vector4(0,180,-1),3 };     //...by creating Menu element
			menu_.AddElement(temp, [&](LambdaStruct struct_) {
				sendLoadLevel(arenaDenominator_);
				struct_.switcher->new_state_ = new MultiplayerArena(platform_, input_, audio_, font_, sprite_renderer_, renderer_3D_, audio_3D_, arenaDenominator_);
				struct_.switcher->needs_to_change = true;
				network.Server->StopListening();
				});
			canPlay_ = true;
		}

		else if (network.Server->GetNumOfClients() == 0 && canPlay_)     //if the last client dsconnects, remove the menu option play
		{
			menu_.RemoveElement("PLAY");
			canPlay_ = false;
		}
	}

	else if (network.role == NetworkRole::Client && network.Client->IsConnected()) 
	{
		network.Client->sendPingIn();                                                               //once the client is waiting for the game to start, keep checking ping...
		ping_ = { std::to_string(network.Client->GetServerPing()),gef::TJ_LEFT,Vector4(0,0,-1),3 }; //...update the value to display...
		menu_.RemoveElement("HOST");                                                                //...remove the other network options
		menu_.RemoveElement("CONNECT");                                                             //...
	}

	for (int i = 0; i<loadCircles_.size();i++)                                                      //update load circles
	{
		if (network.role == NetworkRole::Client ||                                                  //if connection statu is settled, find the difference between current angles and 45 and interpolate to it 
			(network.role == NetworkRole::Host && network.Server->GetNumOfClients() > 0))           //...
		{
			float initialAngle = loadCircles_[i]->getTransform().rotation_.z() > 0 ? (loadCircles_[i]->getTransform().rotation_.z() - 45)*-1 : loadCircles_[i]->getTransform().rotation_.z() + 315;
			loadCircles_[i]->Rotate(Vector4(0, 0, initialAngle * pow(4, (i)) * pow(-1, (i + 1)) * deltaT));
			continue;
		}
		loadCircles_[i]->Rotate(Vector4(0, 0, 50 * pow(4, (i)) * pow(-1, (i + 1)) * deltaT));       //otherwise just keep rotating
	}
}