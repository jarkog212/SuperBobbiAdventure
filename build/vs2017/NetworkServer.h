#pragma once
#pragma warning(disable:4996) 

#include "NetworkUtils.h"
#include <map>
#include <thread>

#define MAXCLIENTS 3               //max number of connected clients, this + 1 gives you the number of players 
#define UDPREADATTEMPTS 5          //how many times to check for a message

class NetworkServer
{
public:
	static NetworkServer* CreateServer();                  //separaate function for creating a server, constructor is private so that there is only one server
	void CleanUpServer();                                  //closes sockets, removes clients, send disconnects and removes any lingering objects
	void ReceiveAllMessages();                             //goes through all available sockets and runs recv on them till failure
	void SendMessagesToAll();                              //send all queued messages to all clients
	void ListenForClients();                               //starts a separate thread to listen for connections
	void StopListening();                                  //stop the separate thread for connection listening
	void SendToAll(std::shared_ptr<Message> toAdd);        //puts a given message into the queue of messages to send
	bool SendMsgTo(Message* msg, int client);              //sends a given message to a single client
	int GetNumOfClients() { return clients.size(); };      //returns number of connected clients (not players)
	void sendPingIn(int ID);                               //sends a ping request for the client to send it back with the same timestamp
	float GetClientPing(int ID) { return pings_[ID]; };    //gets the last stored ping for a given client

private: //change to private later
	NetworkServer();                                       //constructor is private
	~NetworkServer() {};                                   //destructor...
	
	ON_SIMPLE_MESSAGE_FUNCTION_DECLARATION(PingIn)           //declare send and repsponse functions for message types...
		                                          
	ON_SIMPLE_MESSAGE_FUNCTION_DECLARATION(PingOut)          //...
	void sendPingOut(DWORD timestamp, int ID);

	ON_SIMPLE_MESSAGE_FUNCTION_DECLARATION(ClientDisconnect)
	SEND_TYPE_MESSAGE_FUNCTION_DECL(ServerDisconnect)        

	void HandleNetworkingMessages();                         //handle all the networking status messages
	void ReceiveAllMessagesUDP();                            //keeps checking and receiving UDP socket messages until fail
	bool ReceiveObjectMessageUDP();                          //single iteration of recvfrom for UDP socket
	bool ReceiveMessageTCP(SOCKET TCP_in);                   //...              ...recv for TCP socket of a given client 
	void CloseClient(int ID);                                //closes the socket of a given client, with disconnect message
	void RemoveClient(int ID);                               //simply removes the client form the list, part of CloseClient
	bool SendMsgToAll(Message* msg, fd_set& writeable);      //send a given message to all the clients

	int getClientIDFromTCP(SOCKET TCP_in);                   //gets the client ID using the TCP socket
	int getClientIDFromUDP(sockaddr_in UPDaddress);          //...                       ...IP address of client's UDP socket
	void flushAllMessages();                                 //clears all messages, including system ones

	bool isListening = true;                                 //state flag for the listening thread
	std::thread* serverListenThread = NULL;                  //pointer to the listening thread

	WSADATA wsa;                                             //networking objects and winsock...
	SOCKET UDP_serverSock;                                   //...
	SOCKET TCP_serverConnect;
	std::map<int,std::shared_ptr<Client>> clients;           //map of all the clients connected uses ID as key, holds all relevant connection info

	sockaddr_in serverAddrTCP;                                         //adresses to Server
	sockaddr_in serverAddrUDP;                                         //...

	std::queue<std::shared_ptr<Message>> toSend;                       //queue of messages to send
	std::queue<std::shared_ptr<Message>> receivedNetworkingMessages;   //separate queue for received system messages

	timeval time;

	float pings_[MAXCLIENTS] = {0.f};                                  //array of pings to store each clients connectikon lag, currently not used 
};

