#pragma once
#include "NetworkTypes.h"

#pragma warning(disable:4996) 
#pragma comment(lib, "ws2_32.lib")

#define MESSAGEPEEKREAD_UDP sizeof(ObjectAttribute)       //number of bytes to peek for a UDP message, not used
#define MESSAGEPEEKREAD_TCP sizeof(MessageType)           //number of bytes to peek for TCP message, enough for the header

#define MAXSENDATTEMPTS 10                                //number of times to attempt select when sending
#define MAXRECVATTEMPTS 10                                //...                                 ...receiving

#define SERVERIP "127.0.0.1"                              //server adress and ports
#define SERVERPORTTCP 4444
#define SERVERPORTUDP 5555

enum class NetworkRole {                                  //enum of network roles, used to specify the instances networking role
	None,
	Client,
	Host
};

void die(const char* message);                             //Print an error message and exit.

int getAttributeBufferSize(ObjectAttribute type);          //get the size of a subbuffer 
int getMessageBufferSize(MessageType type);                //...            ...buffer for messae type
int getMessageBufferSizeWithTimeStamp(MessageType type);   //...                                   ...and add size of the timestamp

void BuildSimpleRawMessage(Message& msg);                      //serialization function for a message with header and timestamp only
void ReBuildSimpleMessage(Message& msg);                       //deserialization function for a buffer...

void BuildPingOutRawMessage(Message& msg, DWORD timestamp);    //special case of a simple message, does not serialize current time but a given timstamp instead, used for ping
void ReBuildPingOutMessage(Message& msg);                      //standard declaration of deserialization for simple message, deparate due to macros not being made for this scenario

ALL_BUILDER_FUNCTIONS_FOR_MESSAGETYPES_DECL                         //macro to essentially result in builder function declarations for message types, defined in NetworkTypes and NewtorkMakros

BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE_DECL(ServerConnect)       //separate builder declarations for system message types
BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE_DECL(ClientDisconnect)    //...
BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE_DECL(ServerFull)
BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE_DECL(ServerDisconnect)
BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE_DECL(PingIn)

DWORD AddTimeStamp(std::vector<char>& rawData);                     //function that adds a serialized timestamp to a given buffer
DWORD DeserializeTimeStamp(std::vector<char>& rawData);             //...        ...deserializes a timestamp form a given buffer

//----------------------------Serialization functions, defined here as they're templates
template<typename type>
std::vector<char> SerializeData(type data)               //simple serializaion function for fundamental types and/or fixed sized structs, returns vector of chars (better than pure const char*)
{
	std::vector<char> out;
	out.resize(sizeof(type));
	memcpy(out.data(), &data, sizeof(type));
	return out;
}

template<typename type>
type DeserializeData(const char* data)                   //simple deserialization function for fundamental types from a given buffer
{
	type out;
	memcpy((type*)&out, data, sizeof(type));
	return out;
}

template<typename subtype>
std::vector<subtype> DeserializeAttributeVector(const char* rawData, int vectorSize)   //deserialization used for an object's attribute, deserializes the data as a vector of multiple vals ex. position is 2
{
	std::vector<subtype> out;
	std::vector<char> toDeserialize;
	toDeserialize.resize(sizeof(subtype) * vectorSize);
	memcpy(toDeserialize.data(), rawData + sizeof(ObjectAttribute), sizeof(subtype) * vectorSize);

	for (int i = 0; i < vectorSize; i++)
	{
		subtype toAdd = DeserializeData<subtype>(toDeserialize.data() + sizeof(subtype) * i);
		out.push_back(toAdd);
	}
	return out;
}

template<typename subtype>
std::vector<char> SerializeAttributeVector(ObjectAttribute type, std::vector<subtype>& data)  //serialization function for vector of fundamental type. Size doesnt matter, however its not saved into the buffer...
{                                                                                             //information is linked with the attribute type itself.
	std::vector<char> out;
	out.resize(sizeof(ObjectAttribute) + sizeof(subtype) * data.size());

	memcpy(out.data(), SerializeData<ObjectAttribute>(type).data(), sizeof(ObjectAttribute));
	int currentIndex = sizeof(ObjectAttribute);

	for (auto it : data)
	{
		std::vector<char> element = SerializeData<subtype>(it);
		memcpy(out.data() + currentIndex, element.data(), sizeof(subtype));
		currentIndex += sizeof(subtype);
	}

	return out;
}

template<typename subtype>
subtype DeserializeAttribute(const char* rawData)                                            //Deserialization function for an attribute of an object, ignores the header of the subbuffer (attribute enum)
{
	std::vector<char> toDeserialize;
	toDeserialize.resize(sizeof(subtype)) ;
	memcpy(toDeserialize.data(), rawData + sizeof(ObjectAttribute), sizeof(subtype));
	return DeserializeData<subtype>(toDeserialize.data());
}

template<typename subtype>
std::vector<char> SerializeAttribute(ObjectAttribute type, subtype& data)                     //serializes an object's attribute, first the attribute enum then the actual data
{
	std::vector<char> out;
	out.resize(sizeof(ObjectAttribute) + sizeof(subtype)) ;
	memcpy(out.data(), SerializeData<ObjectAttribute>(type).data(), sizeof(ObjectAttribute));
	int currentIndex = sizeof(ObjectAttribute);
	memcpy(out.data() + currentIndex, SerializeData<subtype>(data).data(), sizeof(subtype));
	return out;
}