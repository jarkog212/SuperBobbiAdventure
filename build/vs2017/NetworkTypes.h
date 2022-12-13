#pragma once
#include "NetworkMakros.h"
#include <string>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <list>
#include <memory>
#include <queue>
#include <winsock2.h>

enum class ObjectAttribute : int            // attributes being sent about a single object through UDP
{
	ID = 0,
	Position,
	Rotation,
	Health,
	Velocity,
	State,
	Data                                     // wild-card attribute, not used  
};

enum class MessageType : int                 // all the message types, to add more just add them here and to the lists below, complex messages would need custom builders
{
	                                         //networking messages
	ServerFull = 0,
	ServerConnect,
	ServerDisconnect,
	ClientDisconnect,
	PingIn,
	PingOut,
	                                         //complex messages - require specific building and rebuilding
	Chat,                                    //not used                   
	ObjectData,
	LoadLevel,
	SpawnObject,
	                                         //simple messages - use generated building and rebuilding
	StartGame,
	PlayerLost,
	ClientReady,
	GameEnd
};

struct Client                                // struct holding all the information about client's connection
{
	int ID = -1;
	sockaddr_in TCPaddress;
	SOCKET TCP_sock;
	sockaddr_in UDPaddress;
};

struct Message                               //default message structure
{
	int M_clientID = -1;
	MessageType M_type;
	DWORD M_timeStamp;
	std::vector<char> M_rawData;             //the packet/buffer that gets sent or received
};

STRUCT_OF_MESSAGETYPE(Object)                //macro ensures standardized naming within the code, structure of the UDP object message
{
	int M_objectID = -1;
	std::vector<double> M_position;
	std::vector<double> M_velocity; 
	double M_rotationZ;
	int M_health;
	int M_state;
};

STRUCT_OF_MESSAGETYPE(Chat)                   //unused
{
	int M_size;
	std::string M_user;
	std::string M_text;
};

STRUCT_OF_MESSAGETYPE(LoadLevel)               //message instruction to load a new level from file.
{
	std::string M_level_denominator;           //level to load, name of the level
};

STRUCT_OF_MESSAGETYPE(SpawnObject)             //spawn request for the server, uses data to spawn a desired object
{
	int M_enum_major;                          //object type, enum value
	int M_enum_minor;                          //object sub type if available, enum value
	int M_networkID;                           //server assigned network ID, used to match objects of multiple simulations
	std::vector<double> M_position;
	std::vector<double> M_data;
};

//creates all the reception queues for message types as well as their accessor methods, macros ensure a standardized naming and simplicity
#define ALL_MESSAGETYPE_QUEUES                \
	QUEUE_FOR_MESSAGETYPE(LoadLevel)          \
	QUEUE_FOR_MESSAGETYPE(Object)             \
    QUEUE_FOR_MESSAGETYPE(SpawnObject)        \
    QUEUE_FOR_SIMPLE_MESSAGETYPE(StartGame)   \
	QUEUE_FOR_SIMPLE_MESSAGETYPE(PlayerLost)  \
	QUEUE_FOR_SIMPLE_MESSAGETYPE(GameEnd)     \
	QUEUE_FOR_SIMPLE_MESSAGETYPE(ClientReady)

//creates the message builder functions declarations 
#define ALL_BUILDER_FUNCTIONS_FOR_MESSAGETYPES_DECL              \
	BUILDER_FUNCTIONS_FOR_MESSAGE_TYPE(Object)                   \
	BUILDER_FUNCTIONS_FOR_MESSAGE_TYPE(LoadLevel)                \
	BUILDER_FUNCTIONS_FOR_MESSAGE_TYPE(SpawnObject)              \
    BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE_DECL(StartGame)    \
    BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE_DECL(PlayerLost)   \
	BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE_DECL(GameEnd)      \
	BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE_DECL(ClientReady)

//creates the definitions of builder functions for simple types
#define ALL_BUILDER_FUNCTIONS_FOR_MESSAGETYPES              \
	BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE(StartGame)    \
    BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE(PlayerLost)   \
	BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE(GameEnd)      \
	BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE(ClientReady)

//all cases within the sorting function, used to sort messages into their respective receive buffers
#define ALL_CASSES_FOR_MESSAGE_TYPE           \
	CASE_FOR_MESSAGETYPE(LoadLevel)           \
    CASE_FOR_MESSAGETYPE(SpawnObject)         \
	CASE_FOR_SIMPLE_MESSAGETYPE(StartGame)    \
	CASE_FOR_SIMPLE_MESSAGETYPE(PlayerLost)   \
	CASE_FOR_SIMPLE_MESSAGETYPE(GameEnd)      \
    CASE_FOR_SIMPLE_MESSAGETYPE(ClientReady)

//adds all the flushing loops for all message types 
#define FLUSH_ALL_MESSAGE_TYPES                 \
	FLUSH_MESSAGES_OF_MESSAGETYPE(Object)       \
	FLUSH_MESSAGES_OF_MESSAGETYPE(LoadLevel)    \
	FLUSH_MESSAGES_OF_MESSAGETYPE(SpawnObject)  \
	FLUSH_MESSAGES_OF_MESSAGETYPE(StartGame)    \
	FLUSH_MESSAGES_OF_MESSAGETYPE(PlayerLost)   \
	FLUSH_MESSAGES_OF_MESSAGETYPE(GameEnd)      \
	FLUSH_MESSAGES_OF_MESSAGETYPE(ClientReady)

//declarations for all message handlers for all types
#define ALL_ON_MESSAGE_FUNCTIONS_DECLARATIONS           \
	ON_MESSAGE_FUNCTION_DECLARATION(Object)             \
	ON_MESSAGE_FUNCTION_DECLARATION(SpawnObject)        \
	ON_SIMPLE_MESSAGE_FUNCTION_DECLARATION(StartGame)   \
	ON_SIMPLE_MESSAGE_FUNCTION_DECLARATION(PlayerLost)  \
	ON_SIMPLE_MESSAGE_FUNCTION_DECLARATION(GameEnd)     \
	ON_SIMPLE_MESSAGE_FUNCTION_DECLARATION(ClientReady)

//all checks for message availability and then calls for the respective handler, used within the handle network functions to simplify that section's code, received message as argument
#define ALL_ON_MESSAGE_CALLS                          \
	CALL_MESSAGE_TYPE_HANDLER_FUNCTION(SpawnObject)   \
	CALL_MESSAGE_TYPE_HANDLER_FUNCTION(Object)        \
	CALL_MESSAGE_TYPE_HANDLER_FUNCTION(StartGame)     \
	CALL_MESSAGE_TYPE_HANDLER_FUNCTION(PlayerLost)    \
	CALL_MESSAGE_TYPE_HANDLER_FUNCTION(GameEnd)       \
	CALL_MESSAGE_TYPE_HANDLER_FUNCTION(ClientReady)

//all declarations of send functions per message type, simple messages also have implementation
#define ALL_SEND_TYPE_MESSAGES                                     \
	SEND_TYPE_MESSAGE_FUNCTION_DECL(Object, GameObject_3D* object) \
	SEND_TYPE_MESSAGE_FUNCTION_DECL(SpawnObject)                   \
	SEND_SIMPLE_TYPE_MESSAGE_FUNCTION(StartGame)                   \
	SEND_SIMPLE_TYPE_MESSAGE_FUNCTION(PlayerLost)                  \
	SEND_SIMPLE_TYPE_MESSAGE_FUNCTION(GameEnd)                     \
	SEND_SIMPLE_TYPE_MESSAGE_FUNCTION(ClientReady)