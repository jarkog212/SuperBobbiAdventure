#pragma once
#include "NetworkGlobals.h"

extern Network network;

NetworkClient* NetworkClient::CreateClient()
{
	if (network.role == NetworkRole::None) {  //check whether this running instance is already assigned a role
		auto out = new NetworkClient();       //call the private constructor...
		network.Client = out;                 //...store this object in the global space...
		out->ConnectToServer();               //... try to connect to server
		if (!out->connectionState_)
		{
			out->cleanUpClient();             //cleanup this client instance if fail
			return NULL;
		}
		network.role = NetworkRole::Client;   //set our network role
		return out;
	}
	return NULL;
}

void NetworkClient::cleanUpClient()
{
	closesocket(UDP_sock);                      //UDP socket can simply be closed
	sendClientDisconnect();                     //send a disconnect message to server
	shutdown(TCP_sock, SD_SEND);                //send an end message to server, probably redundant
	closesocket(TCP_sock);                      //close TCP
	
	flushAllMessages();                         //then cleanup...

	WSACleanup();                               //...
	network.Client = NULL;
	network.role = NetworkRole::None;
	delete this;
}

NetworkClient::NetworkClient()
{
	int error = WSAStartup(0x0202, &wsa);               //setup winsock
	if (error != 0)
	{
		die("WSAStartup failed");
	}
	if (wsa.wVersion != 0x0202)
	{
		die("Wrong WinSock version");
	}

	UDP_sock = socket(AF_INET, SOCK_DGRAM, 0);           //create an UDP socket
	if (UDP_sock == INVALID_SOCKET)
		die("socket failed");

	// Create a TCP socket
	TCP_sock = socket(AF_INET, SOCK_STREAM, 0);          //create a TCP socket
	if (TCP_sock == INVALID_SOCKET)
		die("socket failed");

	time.tv_sec = 0;
	time.tv_usec = 10;
}

bool NetworkClient::ReceiveObjectMessageUDP()
{
	ObjectMessage receivedMsg;                                                                                  //create and resize the buffer for object packet
	receivedMsg.M_rawData.resize(getMessageBufferSizeWithTimeStamp(MessageType::ObjectData));

	int count = recvfrom(UDP_sock, receivedMsg.M_rawData.data(), receivedMsg.M_rawData.size(), 0, NULL, NULL);  //receive from server, hence no need to track from address
	if (count < 0)
		return false;
	if (count != receivedMsg.M_rawData.size())
		return false;

	ReBuildObjectMessage(receivedMsg);                                                   //deserialize and store the message in the correct queue
	network.receivedObjectMessages.push(std::make_shared<ObjectMessage>(receivedMsg));
	return true;
}

bool NetworkClient::ReceiveMessageTCP(SOCKET TCP_in)                             //similar to Server's version of this function, could probably be put in utils
{
	std::vector<char> recvBuffer;
	recvBuffer.resize(sizeof(MessageType));

	int count = recv(TCP_in, recvBuffer.data(), MESSAGEPEEKREAD_TCP, MSG_PEEK);
	if (count == 0)
	{
		Message msg;
		msg.M_type = MessageType::ServerDisconnect;
		receivedNetworkingMessages.push(std::make_shared<Message>(msg));
	}
	if (count < 0)
		return false;
	if (count != sizeof(MessageType))
		return false;

	MessageType type = DeserializeData<MessageType>(recvBuffer.data());

	recvBuffer.clear();
	recvBuffer.resize(getMessageBufferSizeWithTimeStamp(type));

	count = recv(TCP_in, recvBuffer.data(), recvBuffer.size(), 0);
	if (count == 0)
	{
		Message msg;
		msg.M_type = MessageType::ServerDisconnect;
		receivedNetworkingMessages.push(std::make_shared<Message>(msg));
	}
	if (count < 0)
		return false;
	if (count != recvBuffer.size())
		return false;

	network.SortMessage(type, recvBuffer, &receivedNetworkingMessages);

	return true;
}

void NetworkClient::HandleNetworkingMessages()           //similar to Server's handler
{
	while (!receivedNetworkingMessages.empty())
	{
		auto msg = receivedNetworkingMessages.front();
		receivedNetworkingMessages.pop();

		switch (msg->M_type) 
		{
		case MessageType::ServerDisconnect:              //different from server, server sends server disconnect not client disconnect       
			onServerDisconnectMessage(msg);
			break;
		case MessageType::PingIn:
			onPingInMessage(msg);
			break;
		case MessageType::PingOut:
			onPingOutMessage(msg);
			break;
		}
	}
}

//---------------handler functions for messages
void NetworkClient::onServerDisconnectMessage(std::shared_ptr<Message>& msg) 
{
	connectionState_ = false;
}

void NetworkClient::onPingInMessage(std::shared_ptr<Message>& msg) 
{
	sendPingOut(msg->M_timeStamp);
}

void NetworkClient::onPingOutMessage(std::shared_ptr<Message>& msg)
{
	previousPing_ = ping_;
	ping_ = ((float)(timeGetTime() - msg->M_timeStamp) / 1000.000f) / 2.0f;
	if (previousPing_ < 0)
		previousPing_ = ping_;
	ping_ = (previousPing_ + ping_) / 2.0f; 
}

//------------------------sender functions
void NetworkClient::sendClientDisconnect()
{
	Message msg;
	BuildClientDisconnectRawMessage(msg);
	network.Client->SendMsg(&msg);
};

void NetworkClient::sendPingIn()
{
	Message msg;
	BuildPingInRawMessage(msg);
	network.Client->SendMsg(&msg);
};

void NetworkClient::sendPingOut(DWORD timestamp)
{
	Message msg;
	BuildPingOutRawMessage(msg, timestamp);
	network.Client->SendMsg(&msg);
};

void NetworkClient::flushAllMessages()
{
	network.flushAllPublicMessages();                   //flush all the public global message queues

	while (!receivedNetworkingMessages.empty())         //flush the internal system message queue
		receivedNetworkingMessages.pop();
}

void NetworkClient::ReceiveAllMessages()                       //similar to Serves's version of this function
{
	fd_set readable;                                           //setup for a select
	FD_ZERO(&readable);                                        //...
	FD_SET(TCP_sock, &readable);                            
	FD_SET(UDP_sock, &readable);

	bool receivedMessage = true;                               //flag for the loop
	while (receivedMessage)                                    //keep receiving if able to
	{
		receivedMessage = false;
		select(0, &readable, NULL, NULL, &time);
		if (FD_ISSET(UDP_sock, &readable) != 0)
			receivedMessage = ReceiveObjectMessageUDP();

		if (FD_ISSET(TCP_sock, &readable) != 0)
			receivedMessage = ReceiveMessageTCP(TCP_sock);
	}

	HandleNetworkingMessages();
	
	if (!connectionState_)                  //added check for cleanup, disconnected client should not exist
		cleanUpClient();
}

bool NetworkClient::SendMsg(Message* msg)                    //similar to Server's implementation, TCP send immediately and only to Server, does not iterate over other clients
{
	fd_set writeable;
	FD_ZERO(&writeable);

	if (msg->M_type == MessageType::ObjectData)
	{
		FD_SET(UDP_sock, &writeable);
		select(0, NULL, &writeable, NULL, &time);

		if (FD_ISSET(UDP_sock, &writeable) == 0)
			return false;

		else
		{
			if (sendto(UDP_sock, msg->M_rawData.data(), msg->M_rawData.size(), 0, (const sockaddr*)&serverAddrUDP, sizeof(serverAddrUDP)) != msg->M_rawData.size())
				return false;
		}
		return true;
	}
	else
	{
		FD_SET(TCP_sock, &writeable);
		select(0, NULL, &writeable, NULL, &time);

		if (FD_ISSET(TCP_sock, &writeable) == 0)
			return false;

		else 
		{
			if (send(TCP_sock, msg->M_rawData.data(), msg->M_rawData.size(), 0) != msg->M_rawData.size())
				return false;
			return true;
		}
	}
}

void NetworkClient::ConnectToServer()                            //connection protocol function for Client
{
	fd_set readable;
	Message shake;

	auto checkShake = [&] {                                       //function that is used only here, checksfor a handshake from server, probably no needed but used as another layer of stability
		bool success = false;
		for (int i = 0; i < MAXREADATTEMPTS && !success; i++) 
		{
			Sleep(50);                                            //give server some time to send the message
			FD_ZERO(&readable);
			FD_SET(TCP_sock, &readable);

			select(0, &readable, NULL, NULL, &time);

			if (FD_ISSET(TCP_sock, &readable) != 0)
			{
				//std::cout << "TCP shake success" << std::endl;
				success = true;
			}
		}
		
		if(!success)                                              //exit the protocol and close the socket if fail
		{
			//std::cout << "fail TCP response" << std::endl;
			closesocket(TCP_sock);
			return false;
		}

		shake.M_rawData.resize(getMessageBufferSizeWithTimeStamp(MessageType::ServerConnect));      //prepare a buffer to receive a message and check the type...
		if (recv(TCP_sock, shake.M_rawData.data(), shake.M_rawData.size(), 0) == 0)  
		{
			//std::cout << "TCP signalled end" << std::endl;
			closesocket(TCP_sock);                                                                  //...exit the protocol and close the socket if fail
			return false;
		}

		ReBuildSimpleMessage(shake);
		
		if(serverTimeOffset_ == 0)                                                                  //save this offset directly if this is the first one
			serverTimeOffset_ = timeGetTime() - shake.M_timeStamp;                                  //finds offset without considering ping, not fully reliable
		else
		{
			serverTimeOffset_ += timeGetTime() - shake.M_timeStamp;                                 //uses previous pings to better estimate the new one, gets rid of sudden extremes
			serverTimeOffset_ /= 2;
		}

		return true;
	};

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serverAddr.sin_port = htons(SERVERPORTTCP);

	if (connect(TCP_sock, (const sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)        //try to connect to server
		return;

	if (!checkShake())                                                                              //check for a handshake form server
		return;

	if (shake.M_type != MessageType::ServerConnect || shake.M_type == MessageType::ServerFull)      
	{
		//std::cout << "fail TCP = full server" << std::endl;
		closesocket(TCP_sock);
		return;
	}

	//std::cout << "TCP connected" << std::endl; 

	serverAddrUDP.sin_family = AF_INET;
	serverAddrUDP.sin_addr.s_addr = inet_addr(SERVERIP);
	serverAddrUDP.sin_port = htons(SERVERPORTUDP);
	
	Message msg;                                                                                    //prepare and later send the connection message
	BuildServerConnectRawMessage(msg);

	//Fine as there wont be any join in progress
	if (sendto(UDP_sock, msg.M_rawData.data(), msg.M_rawData.size(), 0, (const sockaddr*)&serverAddrUDP, sizeof(serverAddrUDP)) != msg.M_rawData.size())
	{
		//std::cout << "fail send UDP" << std::endl;
		closesocket(TCP_sock);
		return;
	}

	if (!checkShake())                                                                              //another checking for a handshake from client
		return;

	if (shake.M_type != MessageType::ServerConnect)
	{
		//td::cout << "TCP fail - UDP" << std::endl;
		closesocket(TCP_sock);
		return;
	}

	//std::cout << "end" << std::endl; //debug

	connectionState_ = true;                                                                        //connection established
}
