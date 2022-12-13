#pragma once
#include "NetworkUtils.h"
#pragma warning(disable:4996) 

#define MAXREADATTEMPTS 5                                        //how many times you select should fail before leaving it

class NetworkClient
{
public:
	static NetworkClient* CreateClient();                        //create a client object, used instead of the constructor. Only allows for one client instance and no server
	void cleanUpClient();                                        //cleans up the client object and send client disconnect to server
	void ReceiveAllMessages();                                   //receive all messages that you can, until select would fail
	bool SendMsg(Message* msg);                                  //send a message to server
	void ConnectToServer();                                      //function to connect to server
	bool IsConnected() { return connectionState_; };             //return client's connection status
	float GetServerPing() { return ping_; }                      //returns averaged current ping to server

	SEND_TYPE_MESSAGE_FUNCTION_DECL(PingIn)                      //send function for ping, public so that it can be sent at any time

private: 
	NetworkClient();                                             //private constructor
	~NetworkClient() {};                                         //...  ...destructor

	ON_SIMPLE_MESSAGE_FUNCTION_DECLARATION(ServerDisconnect)     //message send and receive handlers declarations
	SEND_TYPE_MESSAGE_FUNCTION_DECL(ClientDisconnect)            //...

	ON_SIMPLE_MESSAGE_FUNCTION_DECLARATION(PingIn)

	ON_SIMPLE_MESSAGE_FUNCTION_DECLARATION(PingOut)
	void sendPingOut(DWORD timestamp);
	
	bool ReceiveObjectMessageUDP();                              //try to receive a single UDP message
	bool ReceiveMessageTCP(SOCKET TCP_in);                       //try to receive a single TCP message
	void HandleNetworkingMessages();                             //general update for networking of the client

	void flushAllMessages();                                     //flush all messages, including the system ones

	WSADATA wsa;                                                     //networking objects
	SOCKET UDP_sock;                                                 //...
	SOCKET TCP_sock;
	sockaddr_in serverAddr;
	sockaddr_in serverAddrUDP;

	std::queue<std::shared_ptr<Message>> receivedNetworkingMessages;       //received system messages

	timeval time;                          
	DWORD serverTimeOffset_ = 0;       //the difference between time of the server and client, used for syncing
	float ping_ = -1.f;                //ping to server (in s)
	float previousPing_ = -1.0;        //previous ping t server, used for averaging, smoothing and preventing quick jumps
	bool connectionState_ = false;     //state of connection to server, if this object exists this should not be 'false'
};

