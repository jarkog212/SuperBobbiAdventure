#pragma once

//case for the switch case in the sort function, pushes the message into the correct received queue, for complex message types
#define CASE_FOR_MESSAGETYPE(type)                                                \
	case MessageType::##type:                                                     \
	{                                                                             \
		type##Message msg;                                                        \
		msg.M_rawData.resize(recvBuffer.size());                                  \
		memcpy(msg.M_rawData.data(), recvBuffer.data(), recvBuffer.size());       \
		ReBuild##type##Message(msg);                                              \
		msg.M_clientID = clientID;                                                \
		received##type##Messages.push(std::make_shared<##type##Message>(msg));    \
	}                                                                             \
	break;                                                                                

//case for the switch case in the sort function, pushes the message into the correct received queue, for simple types
#define CASE_FOR_SIMPLE_MESSAGETYPE(type)                                 \
	case MessageType::##type:                                             \
	{                                                                     \
		Message msg;                                                      \
		msg.M_rawData.resize(recvBuffer.size());                          \
		memcpy(msg.M_rawData.data(),recvBuffer.data(), recvBuffer.size());\
		ReBuild##type##Message(msg);                                      \
		msg.M_clientID = clientID;                                        \
		received##type##Messages.push(std::make_shared<Message>(msg));    \
	}                                                                     \
	break;   

//declaration for the queue for a given message type, along with accessor declarations/definitions, for complex message types
#define QUEUE_FOR_MESSAGETYPE(type)                                                                                                                    \
	private: std::queue<std::shared_ptr<##type##Message>> received##type##Messages;                                                                    \
	public: bool gotAny##type##Message() { return !received##type##Messages.empty(); }                                                                 \
	int count##type##lMessages() { return received##type##Messages.size(); };                                                                          \
	std::shared_ptr<##type##Message> get##type##Message() { auto out = received##type##Messages.front(); received##type##Messages.pop(); return out; }

//declaration for the queue for a given message type, along with accessor declarations/definitions, for simple message types
#define QUEUE_FOR_SIMPLE_MESSAGETYPE(type)                                                                                                             \
	private: std::queue<std::shared_ptr<Message>> received##type##Messages;                                                                            \
	public: bool gotAny##type##Message() { return !received##type##Messages.empty(); }                                                                 \
	int count##type##lMessages() { return received##type##Messages.size(); };                                                                          \
	std::shared_ptr<Message> get##type##Message() { auto out = received##type##Messages.front(); received##type##Messages.pop(); return out; }

//code snippet (not a function) to clear the receive queue of a given message type
#define FLUSH_MESSAGES_OF_MESSAGETYPE(type)    \
	while (!received##type##Messages.empty())  \
		received##type##Messages.pop();
  
//base declaration snippet for complex message structure, used to ensure a standardized naming
#define STRUCT_OF_MESSAGETYPE(type)       \
	struct type##Message : public Message

//declaration for builder functions of a complex message type, to be put in header (.h) files
#define BUILDER_FUNCTIONS_FOR_MESSAGE_TYPE(type)        \
	void Build##type##RawMessage(##type##Message& out); \
	void ReBuild##type##Message(##type##Message& out);

//declaration for builder functions of a simple message type, to be put in header (.h) files
#define BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE_DECL(type)           \
	void Build##type##RawMessage(Message& out); \
	void ReBuild##type##Message(Message& out);

//definitions for builder functions of a simple message type, to be put in source (.cpp) files
#define BUILDER_FUNCTIONS_FOR_SIMPLE_MESSAGE_TYPE(type) \
	void Build##type##RawMessage(Message& out) {        \
		out.M_type = MessageType::##type##;             \
		BuildSimpleRawMessage(out);                     \
		};                                              \
	void ReBuild##type##Message(Message& out) {         \
		ReBuildSimpleMessage(out);                      \
		};

//declaration for handler functions of a complex message type, to be put in header (.h) files
#define ON_MESSAGE_FUNCTION_DECLARATION(type)                       \
    void on##type##Message(std::shared_ptr<##type##Message>& msg);

// declaration for handler functions of a simple message type, to be put in header(.h) files
#define ON_SIMPLE_MESSAGE_FUNCTION_DECLARATION(type)        \
    void on##type##Message(std::shared_ptr<Message>& msg);

// code snippet to iterate through received queue and call handler for every message, not a funvtion()
#define CALL_MESSAGE_TYPE_HANDLER_FUNCTION(type)       \
	while (network.gotAny##type##Message()) {          \
		auto msg = network.get##type##Message();       \
		on##type##Message(msg);                        \
	};

//definition for send functions of a simple message type, to be put in source (.cpp) files
#define SEND_SIMPLE_TYPE_MESSAGE_FUNCTION(type)                       \
	void send##type##() {                                             \
		Message msg;                                                  \
		Build##type##RawMessage(msg);                                 \
		if (network.role == NetworkRole::Host)                        \
		network.Server->SendToAll(std::make_shared<Message>(msg));    \
		else if (network.role == NetworkRole::Client)                 \
		network.Client->SendMsg(&msg);                                \
	};

// declaration for send functions of a complex message type, to be put in header (.h) files, accepts multiple params to be used in the declaration
#define SEND_TYPE_MESSAGE_FUNCTION_DECL(type,...)    \
	void send##type##(__VA_ARGS__);
