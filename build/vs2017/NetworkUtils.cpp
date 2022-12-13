#pragma once
#include "NetworkGlobals.h"
#include <time.h>
#include <cassert>

Network network;                                             //declaring a global networking object

ALL_BUILDER_FUNCTIONS_FOR_MESSAGETYPES                       //declare and define all simple message builders, uses macros

BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE(ServerConnect)     //...             ...message builders for system message type, uses macros
BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE(ClientDisconnect)  //...
BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE(ServerFull)
BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE(ServerDisconnect)
BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE(PingIn)

void die(const char* message) //code by lecturers
{
	fprintf(stderr, "Error: %s (WSAGetLastError() = %d)\n", message, WSAGetLastError());

#ifdef _DEBUG
	// Debug build -- drop the program into the debugger.
	abort();
#else
	exit(1);
#endif
}

int getAttributeBufferSize(ObjectAttribute type)
{
	switch (type)                                               //use type to get a object attribute subbuffer size, predefined constants
	{
	case ObjectAttribute::Data:
		return sizeof(ObjectAttribute) + sizeof(double) * 4;
	case ObjectAttribute::Position:
	case ObjectAttribute::Velocity:
		return sizeof(ObjectAttribute) + sizeof(double) * 2;
	case ObjectAttribute::Rotation:
		return sizeof(ObjectAttribute) + sizeof(double);
	case ObjectAttribute::Health:
	case ObjectAttribute::ID:
		return sizeof(ObjectAttribute) + sizeof(int);           //enums are ints
	case ObjectAttribute::State:
		return sizeof(ObjectAttribute) + sizeof(int);           //enums are ints
	}
}

int getMessageBufferSize(MessageType type)
{
	switch (type)                                               //use type to get message buffer size, predefined constants
	{
	case MessageType::ObjectData:
		return sizeof(ObjectAttribute) * 6 + sizeof(double) * 5 + sizeof(int) * 3;
	case MessageType::ServerConnect:
	case MessageType::ServerFull:
	case MessageType::ServerDisconnect:
	case MessageType::ClientDisconnect:
	case MessageType::PingOut:
	case MessageType::PingIn:
	case MessageType::StartGame:
	case MessageType::PlayerLost:
	case MessageType::ClientReady:
	case MessageType::GameEnd:
		return sizeof(MessageType);
	case MessageType::LoadLevel:
		return sizeof(MessageType) + sizeof(char) * 4;
	case MessageType::SpawnObject:
		return sizeof(MessageType) + sizeof(int) * 3 + getAttributeBufferSize(ObjectAttribute::Position) + getAttributeBufferSize(ObjectAttribute::Data);
	}
}

int getMessageBufferSizeWithTimeStamp(MessageType type)
{
	return getMessageBufferSize(type) + sizeof(DWORD);    //adds a size of timestamp to the buffer size of a message
}

void BuildSimpleRawMessage(Message& msg)
{
	msg.M_rawData.resize(getMessageBufferSize(msg.M_type));                                  //resize the message buffer to contain header                            

	int bufferSize = sizeof(MessageType);                                                   
	memcpy(msg.M_rawData.data(), SerializeData<MessageType>(msg.M_type).data(), bufferSize); //serialize and store the message type data into buffer

	msg.M_timeStamp = AddTimeStamp(msg.M_rawData);                                           //add a timestamp to the message
}

void ReBuildSimpleMessage(Message& msg)
{
	msg.M_type = DeserializeData<MessageType>(msg.M_rawData.data());                         //Deserialize type from received buffer
	msg.M_timeStamp = DeserializeTimeStamp(msg.M_rawData);                                   //Deserialize timstamp from received buffer
}

void BuildPingOutRawMessage(Message& msg, DWORD timeStamp) 
{
	msg.M_rawData.resize(getMessageBufferSizeWithTimeStamp(MessageType::PingOut));                        //serialize and store the message type data into buffer

	int attributeSize = sizeof(MessageType);
	memcpy(msg.M_rawData.data(), SerializeData<MessageType>(MessageType::PingOut).data(), attributeSize); //same as simple message
	memcpy(msg.M_rawData.data() + attributeSize, SerializeData<DWORD>(timeStamp).data(), sizeof(DWORD));  //adds timestamp but its not actual time
}

void ReBuildPingOutMessage(Message& msg) 
{
	ReBuildSimpleMessage(msg);                //delegate
}

DWORD DeserializeTimeStamp(std::vector<char>& rawData)
{
	DWORD out = DeserializeData<DWORD>(rawData.data() + (rawData.size() - sizeof(DWORD)));  //timestamp always stored at the end and always the same size
	return out;
}

void BuildObjectRawMessage(ObjectMessage &out)
{
	out.M_type = MessageType::ObjectData;
	out.M_rawData.resize(getMessageBufferSize(MessageType::ObjectData));

	int currentIndex = 0;
	int attributeSize = getAttributeBufferSize(ObjectAttribute::ID);                                                    //get atribute size...
	
	memcpy(out.M_rawData.data(),SerializeAttribute<int>(ObjectAttribute::ID, out.M_objectID).data(), attributeSize);    //...serialize and store the attribute into buffer...
	currentIndex += attributeSize;                                                                                      //...add to offset for memory manipulation

	attributeSize = getAttributeBufferSize(ObjectAttribute::Position);                                                  //repeat previous steps for all attributes
	memcpy(out.M_rawData.data() + currentIndex,SerializeAttributeVector<double>(ObjectAttribute::Position, out.M_position).data(), attributeSize);
	currentIndex += attributeSize;

	memcpy(out.M_rawData.data() + currentIndex,SerializeAttributeVector<double>(ObjectAttribute::Velocity, out.M_velocity).data(), attributeSize);
	currentIndex += attributeSize;

	attributeSize = getAttributeBufferSize(ObjectAttribute::Rotation);
	memcpy(out.M_rawData.data() + currentIndex,SerializeAttribute<double>(ObjectAttribute::Rotation, out.M_rotationZ).data(), attributeSize);
	currentIndex += attributeSize;

	attributeSize = getAttributeBufferSize(ObjectAttribute::Health);
	memcpy(out.M_rawData.data() + currentIndex,SerializeAttribute<int>(ObjectAttribute::Health, out.M_health).data(), attributeSize);
	currentIndex += attributeSize;

	attributeSize = getAttributeBufferSize(ObjectAttribute::State);
	memcpy(out.M_rawData.data() + currentIndex,SerializeAttribute<int>(ObjectAttribute::State, out.M_state).data(), attributeSize);
	
	out.M_timeStamp = AddTimeStamp(out.M_rawData);
}

void ReBuildObjectMessage(ObjectMessage& out)
{
	out.M_type = MessageType::ObjectData;
	out.M_timeStamp = DeserializeTimeStamp(out.M_rawData);

	int currentIndex = 0;                                                                      //ensures we dont overwrite already deseriaized data
	while (currentIndex < out.M_rawData.size()) {
		auto attr = DeserializeData<ObjectAttribute>(out.M_rawData.data() + currentIndex);     //what attribute is next piece of data

		switch (attr) {                                                                        //based on the attribute, deserialize the right amount of bytes into usable data
		case ObjectAttribute::Position:
			out.M_position = DeserializeAttributeVector<double>(out.M_rawData.data() + currentIndex, 2);  
			break;
		case ObjectAttribute::Velocity:
			out.M_velocity = DeserializeAttributeVector<double>(out.M_rawData.data() + currentIndex, 2);
			break;
		case ObjectAttribute::Health:
			out.M_health = DeserializeAttribute<int>(out.M_rawData.data() + currentIndex);
			break;
		case ObjectAttribute::ID:
			out.M_objectID = DeserializeAttribute<int>(out.M_rawData.data() + currentIndex);
			break;
		case ObjectAttribute::Rotation:
			out.M_rotationZ = DeserializeAttribute<double>(out.M_rawData.data() + currentIndex);
			break;
		case ObjectAttribute::State:
			out.M_state = DeserializeAttribute<int>(out.M_rawData.data() + currentIndex);
			break;
		}
		currentIndex += getAttributeBufferSize(attr);				                           //add to index to protect deserialize data from overwrite
	}
}

void BuildLoadLevelRawMessage(LoadLevelMessage& out)                                           //look through previous builder functions to get the idea
{
	out.M_type = MessageType::LoadLevel;
	out.M_rawData.resize(getMessageBufferSize(MessageType::LoadLevel));

	memcpy(out.M_rawData.data(), SerializeData<MessageType>(MessageType::LoadLevel).data(), sizeof(MessageType));
	memcpy(out.M_rawData.data() + sizeof(MessageType), out.M_level_denominator.data(), out.M_rawData.size() - sizeof(MessageType));

	out.M_timeStamp = AddTimeStamp(out.M_rawData);
}

void ReBuildLoadLevelMessage(LoadLevelMessage& out)                                            //look through previous builder functions to get the idea
{
	out.M_type = DeserializeData<MessageType>(out.M_rawData.data());

	out.M_level_denominator.resize(getMessageBufferSize(MessageType::LoadLevel) - sizeof(MessageType));
	memcpy((char*)out.M_level_denominator.data(), out.M_rawData.data() + sizeof(MessageType), getMessageBufferSize(MessageType::LoadLevel) - sizeof(MessageType));
	
	out.M_timeStamp = DeserializeTimeStamp(out.M_rawData);
}

void BuildSpawnObjectRawMessage(SpawnObjectMessage& out)                                       //look through previous builder functions to get the idea
{
	out.M_type = MessageType::SpawnObject;
	out.M_rawData.resize(getMessageBufferSize(MessageType::SpawnObject));

	int offset = 0;
	memcpy(out.M_rawData.data(), SerializeData<MessageType>(MessageType::SpawnObject).data(), sizeof(MessageType));
	offset += sizeof(MessageType);
	memcpy(out.M_rawData.data() + offset, SerializeData<int>(out.M_enum_major).data(), sizeof(int));
	offset += sizeof(int);
	memcpy(out.M_rawData.data() + offset, SerializeData<int>(out.M_enum_minor).data(), sizeof(int));
	offset += sizeof(int);
	memcpy(out.M_rawData.data() + offset, SerializeData<int>(out.M_networkID).data(), sizeof(int));
	offset += sizeof(int);
	memcpy(out.M_rawData.data() + offset, SerializeAttributeVector<double>(ObjectAttribute::Position, out.M_position).data(), getAttributeBufferSize(ObjectAttribute::Position));
	offset += getAttributeBufferSize(ObjectAttribute::Position);
	memcpy(out.M_rawData.data() + offset, SerializeAttributeVector<double>(ObjectAttribute::Data, out.M_data).data(), getAttributeBufferSize(ObjectAttribute::Data));

	out.M_timeStamp = AddTimeStamp(out.M_rawData);
}

void ReBuildSpawnObjectMessage(SpawnObjectMessage& out)                                        //look through previous builder functions to get the idea
{
	out.M_rawData.resize(getMessageBufferSizeWithTimeStamp(MessageType::SpawnObject));
	
	int offset = 0;
	out.M_type = DeserializeData<MessageType>(out.M_rawData.data());
	offset += sizeof(MessageType);
	out.M_enum_major = DeserializeData<int>(out.M_rawData.data() + offset);
	offset += sizeof(int);
	out.M_enum_minor = DeserializeData<int>(out.M_rawData.data() + offset);
	offset += sizeof(int);
	out.M_networkID = DeserializeData<int>(out.M_rawData.data() + offset);
	offset += sizeof(int);
	out.M_position = DeserializeAttributeVector<double>(out.M_rawData.data() + offset, 2);
	offset += getAttributeBufferSize(ObjectAttribute::Position);
	out.M_data = DeserializeAttributeVector<double>(out.M_rawData.data() + offset, 4);

	out.M_timeStamp = DeserializeTimeStamp(out.M_rawData);
}

DWORD AddTimeStamp(std::vector<char>& rawData)                             
{
	DWORD timeStamp = timeGetTime();                                     //gets current time in ms, relative to the device...
	                                                                     //...rolls over to 0 every ~49 days
	std::vector<char> buffer;
	buffer.resize(rawData.size() + sizeof(DWORD));                       //resize the temp buffer to have space for the timestamp...
	memcpy(buffer.data(), rawData.data(), rawData.size());               //...copy the message buffer into new buffer...
	memcpy(buffer.data() + rawData.size(), &timeStamp, sizeof(DWORD));   //...copy the timestamp data at the end of the buffer

	rawData.resize(buffer.size());                                       //...resize the old buffer to fit the temp buffer...
	memcpy(rawData.data(), buffer.data(), buffer.size());                //...copy temp back into the old buffer
	return timeStamp;
}


