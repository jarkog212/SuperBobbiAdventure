#pragma once
#include "NetworkGlobals.h"

extern Network network;

NetworkServer* NetworkServer::CreateServer()
{
	if (network.role == NetworkRole::None) {                    //Create a new network if were still not assigned a role
		auto out = new NetworkServer();                         //only callable here, private
		//std::cout << "created server" << std::endl;           
		network.Server = out;                                   //stores this new instance into global space
		network.role = NetworkRole::Host;                       //sets the role
		return out;
	}
	return NULL;
}

void NetworkServer::CleanUpServer()                             
{
	StopListening();                                            //stops and cleansup the listening thread and other server things...
	closesocket(UDP_serverSock);                                //...
	closesocket(TCP_serverConnect);
	sendServerDisconnect();
	for (auto it : clients) 
		CloseClient(it.first);
	clients.clear();
	WSACleanup();
	network.Server = NULL;
	network.role = NetworkRole::None;
	flushAllMessages();
	delete this;                                                //deletes itself to ensure no memory leaks
}

NetworkServer::NetworkServer()
{
	int error = WSAStartup(0x0202, &wsa);                                                   //Create and setup WSA
	if (error != 0)                                                                       
		die("WSAStartup failed");
	if (wsa.wVersion != 0x0202)
		die("Wrong WinSock version");
   
	UDP_serverSock = socket(AF_INET, SOCK_DGRAM, 0);                                        //Create a UDP socket
	if (UDP_serverSock == INVALID_SOCKET)
		die("socket failed");

	serverAddrUDP.sin_family = AF_INET;                                                     
	serverAddrUDP.sin_addr.s_addr = inet_addr(SERVERIP);
	serverAddrUDP.sin_port = htons(SERVERPORTUDP);

	if (bind(UDP_serverSock, (const sockaddr*)&serverAddrUDP, sizeof(serverAddrUDP)) != 0)  //bind server UDP to a port, required to allows clients to send to it
		die("bind failed");

	TCP_serverConnect = socket(AF_INET, SOCK_STREAM, 0);                                    //Create a TCP socket
	if (TCP_serverConnect == INVALID_SOCKET)
		die("socket failed");

	serverAddrTCP.sin_family = AF_INET;
	serverAddrTCP.sin_addr.s_addr = inet_addr(SERVERIP);
	serverAddrTCP.sin_port = htons(SERVERPORTTCP);
	// Bind the socket to that address and port.
	if (bind(TCP_serverConnect, (const sockaddr*)&serverAddrTCP, sizeof(serverAddrTCP)) != 0) //bind the TCP socket to a port, required for connecting
		die("bind failed");

	if (listen(TCP_serverConnect, 1) != 0)                                                    //start listening for connections
		die("listen failed");

	time.tv_sec = 0;
	time.tv_usec = 10;

	ListenForClients();                                                                       //start the second thread for listening and handling connections
}

void NetworkServer::ReceiveAllMessagesUDP()
{
	fd_set readable;                                    //setup for select
	FD_ZERO(&readable);
	FD_SET(UDP_serverSock, &readable);

	select(0, &readable, NULL, NULL, &time);            //check and start the loop only if there is a message to receive

	while (FD_ISSET(UDP_serverSock, &readable) != 0)
	{
		ReceiveObjectMessageUDP();                      //receive message
		select(0, &readable, NULL, NULL, &time);        //check if the loop is to continue for aanother message
	}
}

bool NetworkServer::ReceiveObjectMessageUDP()
{
	ObjectMessage receivedMsg = ObjectMessage();
	receivedMsg.M_rawData.resize(getMessageBufferSizeWithTimeStamp(MessageType::ObjectData));                                         //resize to store an entire message
	sockaddr_in from;
	int fromSize = sizeof(from);
	int count = recvfrom(UDP_serverSock, receivedMsg.M_rawData.data(), receivedMsg.M_rawData.size(), 0, (sockaddr*)&from, &fromSize); //receive message...
	if (count < 0)
		return false;
	if (count != receivedMsg.M_rawData.size())
		return false;

	ReBuildObjectMessage(receivedMsg);                                                      //...Deserialize the message...
	receivedMsg.M_clientID = getClientIDFromUDP(from);                                      //...associate message with a client...
	network.receivedObjectMessages.push(std::make_shared<ObjectMessage>(receivedMsg));      //...store the message into queue
	return true;
}

bool NetworkServer::ReceiveMessageTCP(SOCKET TCP_in)
{
	std::vector<char> recvBuffer;
	recvBuffer.resize(sizeof(MessageType));                                          //resize the buffer to have enough space to store a peek of a message

	int count = recv(TCP_in, recvBuffer.data(), MESSAGEPEEKREAD_TCP, MSG_PEEK);      //only receive the header, do not delete the message from the card
	if (count == 0)
	{
		Message msg;
		msg.M_type = MessageType::ClientDisconnect;                                  //0 indicates a "shutdown from Client" hence a disconnect signal is added
		receivedNetworkingMessages.push(std::make_shared<Message>(msg));
	}
	if (count < 0)
		return false;
	if (count != sizeof(MessageType))
		return false;

	MessageType type = DeserializeData<MessageType>(recvBuffer.data());              //get message type

	recvBuffer.clear();
	recvBuffer.resize(getMessageBufferSizeWithTimeStamp(type));                      //resize the buffer based on the type to fit the entire message

	count = recv(TCP_in, recvBuffer.data(), recvBuffer.size(), 0);
	if (count == 0) 
	{
		Message msg;
		msg.M_type = MessageType::ClientDisconnect;
		receivedNetworkingMessages.push(std::make_shared<Message>(msg));
	}
	if (count < 0)
		return false;
	if (count != recvBuffer.size())
		return false;

	network.SortMessage(type, recvBuffer, &receivedNetworkingMessages, getClientIDFromTCP(TCP_in)); //sort message into the correct reception queue
	return true;
}

void NetworkServer::ReceiveAllMessages()
{
	fd_set readable;
	FD_ZERO(&readable);

	for (auto it : clients)                                                                //store all available sckets into the list
		FD_SET(it.second->TCP_sock, &readable);
	FD_SET(UDP_serverSock, &readable);

	bool receivedMessage = true;                                                           //declare and initialise the loop
	while (receivedMessage)      
	{
		receivedMessage = false;
		select(0, &readable, NULL, NULL, &time);
		if (FD_ISSET(UDP_serverSock, &readable) != 0) 
			receivedMessage = ReceiveObjectMessageUDP();                                   //receive the UDP essage if possible

		for (auto it : clients)
			if (FD_ISSET(it.second->TCP_sock, &readable) != 0)
				receivedMessage = ReceiveMessageTCP(it.second->TCP_sock);                  //...      ...TCP...    
	}

	HandleNetworkingMessages();                                                            //handle all the received status messages
}

void NetworkServer::SendMessagesToAll()
{
	fd_set writeable;
	int attempts = 0;

	while (!toSend.empty())                            //iterates through sending queue and sends if it ca
	{
		auto msg = toSend.front();                     //gets the message from the queue...
		toSend.pop();                                  //...removes it from the queue...

		attempts = 0;
		while (attempts < MAXSENDATTEMPTS)             //... attempt to send it
		{
			if (SendMsgToAll(msg.get(), writeable))
				break;
			
			attempts++;
		}
	}
}

bool NetworkServer::SendMsgToAll(Message* msg, fd_set &writeable)
{
	FD_ZERO(&writeable);

	if (msg->M_type == MessageType::ObjectData)                   //if this is a Object message send it through UDP
	{
		FD_SET(UDP_serverSock, &writeable);
		select(0, NULL, &writeable, NULL, &time);

		if (FD_ISSET(UDP_serverSock, &writeable) == 0)            //check if it can send
			return false;

		for (auto it : clients)                                   //iterate through clients and sen dthe message to all of them
		{
			auto toAddress = it.second->UDPaddress;
			if (sendto(UDP_serverSock, msg->M_rawData.data(), msg->M_rawData.size(), 0, (const sockaddr*)&toAddress, sizeof(toAddress)) != msg->M_rawData.size())
				return false;
		}
		return true;
	}
	else                                                          //other messages are sent through TCP
	{
		for(auto it : clients)                             
			FD_SET(it.second->TCP_sock, &writeable);

		select(0, NULL, &writeable, NULL, &time);                 //check to which clients we can send

		for (auto it : clients) {                                 //send the messages of you can
			if (FD_ISSET(it.second->TCP_sock, &writeable) == 0)
				return false;

			if (send(it.second->TCP_sock, msg->M_rawData.data(), msg->M_rawData.size(), 0) != msg->M_rawData.size())
				return false;
		}
		return true;
	}
}

bool NetworkServer::SendMsgTo(Message* msg, int client)
{
	fd_set writeable;
	FD_ZERO(&writeable);
	FD_SET(clients[client]->TCP_sock, &writeable);

	select(0, NULL, &writeable, NULL, &time);                  //check whther we can send

	if (FD_ISSET(clients[client]->TCP_sock, &writeable) == 0)
		return false;

	if (send(clients[client]->TCP_sock, msg->M_rawData.data(), msg->M_rawData.size(), 0) != msg->M_rawData.size())
		return false;

	return true;
}

void NetworkServer::CloseClient(int ID)
{
	if (clients.find(ID) != clients.end())
	{
		shutdown(clients[ID]->TCP_sock, SD_SEND);         //sends an end message
		closesocket(clients[ID]->TCP_sock);               //closes the socekt
	}
}

void NetworkServer::HandleNetworkingMessages()
{
	while (!receivedNetworkingMessages.empty())           //if there are any unhandled messages...
	{
		auto msg = receivedNetworkingMessages.front();
		receivedNetworkingMessages.pop();

		switch (msg->M_type)                              //...call their respective handler function
		{
		case MessageType::ClientDisconnect:
			onClientDisconnectMessage(msg);
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

//-----------------------handler functions
void NetworkServer::onClientDisconnectMessage(std::shared_ptr<Message>& msg) 
{
	RemoveClient(msg->M_clientID);
}

void NetworkServer::onPingInMessage(std::shared_ptr<Message>& msg)
{
	sendPingOut(msg->M_timeStamp,msg->M_clientID);      //send a response to the client with their old timstamp, used to calculate roundtrip time and ping
}

void NetworkServer::onPingOutMessage(std::shared_ptr<Message>& msg)
{
	pings_[msg->M_clientID] = ((float)(timeGetTime() - msg->M_timeStamp) / 1000.000f) / 2.0f; //calculate and store ping of a client who sent the message
}

//-----------------------send functions
void NetworkServer::sendServerDisconnect()
{
	Message msg;
	BuildServerDisconnectRawMessage(msg);
	network.Server->SendToAll(std::make_shared<Message>(msg));
};

void NetworkServer::sendPingIn(int ID)
{
	Message msg;
	BuildPingInRawMessage(msg);
	network.Server->SendMsgTo(&msg,ID);
};

void NetworkServer::sendPingOut(DWORD timestamp, int ID)
{
	Message msg;
	BuildPingOutRawMessage(msg, timestamp);
	network.Server->SendMsgTo(&msg,ID);
};


int NetworkServer::getClientIDFromTCP(SOCKET TCP_in)
{
	for (auto it : clients)                  //linear search for the client
		if (it.second->TCP_sock == TCP_in)   
			return it.second->ID;

	return -1;
}

int NetworkServer::getClientIDFromUDP(sockaddr_in UPDaddress)
{
	for (auto it : clients)                  //linear search for the client with custom comparisons
		if (it.second->UDPaddress.sin_addr.S_un.S_addr == UPDaddress.sin_addr.S_un.S_addr &&  
			it.second->UDPaddress.sin_port == UPDaddress.sin_port)
			return it.second->ID;

	return -1;
}

void NetworkServer::flushAllMessages()
{
	network.flushAllPublicMessages();             //flush all the messages stored in the globals scope

	while (!receivedNetworkingMessages.empty())   //flush all the messages stored locally as status messages
		receivedNetworkingMessages.pop();
}

void NetworkServer::ListenForClients()
{
	if (!serverListenThread)                            //ensure there already is not a listening thread
	{
		isListening = true;
		serverListenThread = new std::thread([&] {      //create the thread for listening and store a pointer to it
			fd_set readable;

			while (isListening)
			{
				FD_ZERO(&readable);
				FD_SET(TCP_serverConnect, &readable);

				select(0, &readable, NULL, NULL, &time);      //check whether connect socket, check any new connection

				if (FD_ISSET(TCP_serverConnect, &readable))   //if readeable then new connection is pending
				{
					Message statusMsg;
					Client client;
					int clientSize = sizeof(client.TCPaddress);
					SOCKET clientSocket = accept(TCP_serverConnect, (sockaddr*)&client.TCPaddress, &clientSize); //accept the connection in all cases
					/*if (clientSocket == INVALID_SOCKET)
						printf("accept failed\n");*/
					if (clients.size() >= MAXCLIENTS)                                                   //check whether there is a spot for the new connection
					{
						BuildServerFullRawMessage(statusMsg);                                           //if there is too many clients, let the new connection know...
						send(clientSocket, statusMsg.M_rawData.data(), statusMsg.M_rawData.size(), 0);  //...by sending a message
						//printf("accept refused - too many clients\n");
						closesocket(clientSocket);
						continue;
					}

					BuildServerConnectRawMessage(statusMsg);
					ReceiveAllMessagesUDP();  //empty out the UDP queue on card, posibly define this as a critical section/mutexes

					if (send(clientSocket, statusMsg.M_rawData.data(), statusMsg.M_rawData.size(), 0) != statusMsg.M_rawData.size());
						//printf("message was not sent\n");

					//std::cout << "TCP connect" << std::endl;

					bool success = false;
					for (int i = 0; i < UDPREADATTEMPTS && !success; i++) {  //start waiting and checking for a UDP message by the client
						
						Sleep(50);                                           

						FD_ZERO(&readable);
						FD_SET(UDP_serverSock, &readable);

						select(0, &readable, NULL, NULL, &time);

						if (FD_ISSET(UDP_serverSock, &readable))               
							success = true;
					}
					if (!success)
					{
						//printf("failed UDP connection - nothing received in time\n");  
						shutdown(clientSocket, SD_SEND);                     //send special end message to client if unsuccessful
						closesocket(clientSocket);
						continue;
					}

					//receive the message from UDP
					int fromAddrSize = sizeof(client.UDPaddress);
					Message msg;
					msg.M_rawData.resize(getMessageBufferSizeWithTimeStamp(MessageType::ServerConnect));
					recvfrom(UDP_serverSock, msg.M_rawData.data(), msg.M_rawData.size(), 0, (sockaddr*)&client.UDPaddress, &fromAddrSize);
					
					ReBuildSimpleMessage(msg);                                //rebuild and...
					if (msg.M_type != MessageType::ServerConnect)             //...check whether the message matches expectation
					{
						//printf("failed UDP connection - wrong message\n");
						shutdown(clientSocket, SD_SEND);                      //send special end message to client if unsuccessful
						closesocket(clientSocket);      
						continue;
					}

					BuildServerConnectRawMessage(statusMsg);
					send(clientSocket, statusMsg.M_rawData.data(), statusMsg.M_rawData.size(), 0); //create and send a confirmation message

					for (int i = 0; i < MAXCLIENTS; i++)           //assign the new connection with correctID
						if (clients.find(i) == clients.end())
						{
							client.ID = i;
							break;
						}

					client.TCP_sock = clientSocket;        
					clients[client.ID] = std::make_shared<Client>(client);
					//std::cout << "UDP connect" << std::endl; //debug
				}
			}
		});
	}
}

void NetworkServer::StopListening()
{
	if (serverListenThread)          //if listening thread is active...
	{
		isListening = false;         //...cleanup
		serverListenThread->join();
		delete serverListenThread;
		serverListenThread = NULL;
	}
}

void NetworkServer::SendToAll(std::shared_ptr<Message> toAdd)
{
	toSend.push(toAdd);
}

void NetworkServer::RemoveClient(int ID)
{
	if (clients.find(ID) != clients.end())   //tries to find the client and removes it if it finds
	{
		CloseClient(ID);
		clients.erase(ID);
	}
}
