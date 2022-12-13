#pragma once
#include "NetworkMakros.h"
#include "NetworkServer.h"
#include "NetworkClient.h"

//holds the global variables in an easy container, used by settings and can be accessed anywhere from code, provided extern is used
class Network {
	friend class NetworkServer;
	friend class NetworkClient;

	ALL_MESSAGETYPE_QUEUES

public:
	NetworkRole role = NetworkRole::None;
	NetworkServer* Server = NULL;
	NetworkClient* Client = NULL;

public:
	void flushAllMessagesUDP() {
		FLUSH_MESSAGES_OF_MESSAGETYPE(Object)
	}
	void flushAllPublicMessages() {
		FLUSH_ALL_MESSAGE_TYPES
	};

private:
	void SortMessage(MessageType type, std::vector<char>& recvBuffer, std::queue<std::shared_ptr<Message>>* internalQueue, int clientID = -1)
	{	
		switch (type) {
		case MessageType::ServerConnect:
		case MessageType::ClientDisconnect:
		case MessageType::ServerFull:
		case MessageType::PingIn:
		case MessageType::PingOut:
		{
			Message receivedMsg;
			receivedMsg.M_rawData.resize(recvBuffer.size());
			memcpy(receivedMsg.M_rawData.data(), recvBuffer.data(), recvBuffer.size());
			ReBuildSimpleMessage(receivedMsg);
			receivedMsg.M_clientID = clientID;
			internalQueue->push(std::make_shared<Message>(receivedMsg));
		}
		break;
		ALL_CASSES_FOR_MESSAGE_TYPE
		}
	}
};

